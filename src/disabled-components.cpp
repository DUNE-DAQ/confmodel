#include "confmodel/Application.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/ResourceSetAND.hpp"
#include "confmodel/ResourceSetOR.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/System.hpp"
#include "confmodel/util.hpp"
#include "confmodel/disabled-components.hpp"

#include "logging/Logging.hpp"

#include "test_circular_dependency.hpp"

using namespace dunedaq::conffwk;
using namespace dunedaq::confmodel;

DisabledComponents::DisabledComponents(Configuration& db,
 System* system) :
  m_db(db),
  m_system(system),
  m_num_of_slr_enabled_resources(0),
  m_num_of_slr_disabled_resources(0)
{
  TLOG_DEBUG(2) <<  "construct the object " << (void *)this  ;
  m_db.add_action(this);
}

DisabledComponents::~DisabledComponents()
{
  TLOG_DEBUG(2) <<  "destroy the object " << (void *)this ;
  m_db.remove_action(this);
}

void
DisabledComponents::notify(std::vector<ConfigurationChange *>& /*changes*/) noexcept
{
  TLOG_DEBUG(2) <<  "reset system components because of notification callback on object " << (void *)this ;
  __clear();
}

void
DisabledComponents::load() noexcept
{
  TLOG_DEBUG(2) <<  "reset system components because of configuration load on object " << (void *)this ;
  __clear();
}

void
DisabledComponents::unload() noexcept
{
  TLOG_DEBUG(2) <<  "reset system components because of configuration unload on object " << (void *)this ;
  __clear();
}

void
DisabledComponents::update(const ConfigObject& obj, const std::string& name) noexcept
{
  TLOG_DEBUG(2) <<  "reset system components because of configuration update (obj = " << obj << ", name = \'" << name << "\') on object " << (void *)this ;
  __clear();
}

void
DisabledComponents::reset() noexcept
{
  TLOG_DEBUG(2) <<  "reset disabled by explicit user call" ;
  m_disabled.clear(); // do not clear s_user_disabled && s_user_enabled !!!
}


void
DisabledComponents::disable_children(const ResourceSet& rs)
{
  for (auto & res : rs.get_contains()) {
    disable(*res);
    if (const auto * rs2 = res->cast<ResourceSet>()) {
      disable_children(*rs2);
    }
  }
}

void
DisabledComponents::disable_children(const Segment& segment)
{
  for (auto & app : segment.get_applications()) {
    auto res = app->cast<Component>();
    if (res) {
      disable(*res);
    }
  }
  for (auto & seg : segment.get_segments()) {
    TLOG_DEBUG(6) <<  "disable segment " << seg << " because it's parent segment " << &segment << " is disabled" ;
    disable(*seg);
    disable_children(*seg);
  }
}

void
System::set_disabled(const std::set<const Component *>& objs) const
{
  m_disabled_components.m_user_disabled.clear();

  for (const auto& comp : objs) {
    m_disabled_components.m_user_disabled.insert(comp);
  }
  m_disabled_components.m_num_of_slr_disabled_resources = m_disabled_components.m_user_disabled.size();

  m_disabled_components.reset();
}

void
System::set_enabled(const std::set<const Component *>& objs) const
{
  m_disabled_components.m_user_enabled.clear();

  for (const auto& i : objs) {
    m_disabled_components.m_user_enabled.insert(i);
  }
  m_disabled_components.m_num_of_slr_enabled_resources = m_disabled_components.m_user_enabled.size();

  m_disabled_components.reset();
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace dunedaq {
  ERS_DECLARE_ISSUE_BASE(
    confmodel,
    ReadMaxAllowedIterations,
    AlgorithmError,
    "Has exceeded the maximum of iterations allowed (" << limit << ") during calculation of disabled objects",
    ,
    ((unsigned int)limit)
  )
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  // fill data from resource sets

static void fill(
  const ResourceSet& rs,
  std::vector<const ResourceSetOR *>& rs_or,
  std::vector<const ResourceSetAND *>& rs_and,
  TestCircularDependency& cd_fuse
)
{
  if (const ResourceSetAND * r1 = rs.cast<ResourceSetAND>())
    {
      rs_and.push_back(r1);
    }
  else if (const ResourceSetOR * r2 = rs.cast<ResourceSetOR>())
    {
      rs_or.push_back(r2);
    }

  for (auto & i : rs.get_contains())
    {
      AddTestOnCircularDependency add_fuse_test(cd_fuse, i);
      if (const ResourceSet * rs2 = i->cast<ResourceSet>())
        {
          fill(*rs2, rs_or, rs_and, cd_fuse);
        }
    }
}


  // fill data from segments

static void fill(
  const Segment& s,
  std::vector<const ResourceSetOR *>& rs_or,
  std::vector<const ResourceSetAND *>& rs_and,
  TestCircularDependency& cd_fuse
)
{
  for (auto & app : s.get_applications()) {
    AddTestOnCircularDependency add_fuse_test(cd_fuse, app);
    if (const ResourceSet * rs = app->cast<ResourceSet>()) {
      fill(*rs, rs_or, rs_and, cd_fuse);
    }
  }

  for (auto & seg : s.get_segments()) {
    TLOG_DEBUG(6) << "Filling segment " << seg->UID();
    AddTestOnCircularDependency add_fuse_test(cd_fuse, seg);
    fill(*seg, rs_or, rs_and, cd_fuse);
  }
}


  // fill data from system

static void fill(
  const System& system,
  std::vector<const ResourceSetOR *>& rs_or,
  std::vector<const ResourceSetAND *>& rs_and,
  TestCircularDependency& cd_fuse
)
{
#if 0
  if (const OnlineSegment * onlseg = p.get_OnlineInfrastructure())
    {
      AddTestOnCircularDependency add_fuse_test(cd_fuse, onlseg);
      fill(*onlseg, rs_or, rs_and, cd_fuse);

      // NOTE: normally application may not be ResourceSet, but for some "exotic" cases put this code
      for (auto &a : p.get_OnlineInfrastructureApplications())
        {
          if (const ResourceSet * rs = a->cast<ResourceSet>())
            {
              fill(*rs, rs_or, rs_and, cd_fuse);
            }
        }
    }
#endif

  auto seg = system.get_segment();
  AddTestOnCircularDependency add_fuse_test(cd_fuse, seg);
  fill(*seg, rs_or, rs_and, cd_fuse);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool
Component::disabled(const System& system) const
{
  TLOG_DEBUG( 6) << "System UID: " << system.UID() << " this->UID()=" << UID();
  // fill disabled (e.g. after system changes)

  if (system.m_disabled_components.size() == 0) {
    if (system.get_disabled().empty() && 
        system.m_disabled_components.m_user_disabled.empty()) {
      TLOG_DEBUG( 6) << "System has no disabled components";
      return false;  // the system has no disabled components
    }
    else {
      // get two lists of all system's resource-set-or and resource-set-and
      // also test any circular dependencies between segments and resource sets
      TestCircularDependency cd_fuse("component \'is-disabled\' status", &system);
      std::vector<const ResourceSetOR *> rs_or;
      std::vector<const ResourceSetAND *> rs_and;
      fill(system, rs_or, rs_and, cd_fuse);

      // calculate explicitly and implicitly (nested) disabled components
      {
        std::vector<const Component *> vector_of_disabled;
        vector_of_disabled.reserve(system.get_disabled().size() + system.m_disabled_components.m_user_disabled.size());

        // add user disabled components, if any
        for (auto & i : system.m_disabled_components.m_user_disabled) {
          vector_of_disabled.push_back(i);
          TLOG_DEBUG(6) <<  "disable component " << i->UID() << " because it is explicitly disabled by user" ;
        }

        // add system-disabled components ignoring explicitly enabled by user
        for (auto & i : system.get_disabled()) {
          TLOG_DEBUG(6) <<  "check component " << i->UID() << " explicitly disabled in system" ;

          if (system.m_disabled_components.m_user_enabled.find(i) == system.m_disabled_components.m_user_enabled.end()) {
            vector_of_disabled.push_back(i);
            TLOG_DEBUG(6) <<  "disable component " << i->UID() << " because it is not explicitly enabled in system" ;
          }
          else {
            TLOG_DEBUG(6) <<  "skip component " << i->UID() << " because it is enabled by user" ;
          }
        }

        // fill set of explicitly and implicitly (segment/resource-set containers) disabled components
        for (auto & i : vector_of_disabled) {
          system.m_disabled_components.disable(*i);

          if (const ResourceSet * rs = i->cast<ResourceSet>()) {
            system.m_disabled_components.disable_children(*rs);
          }
          else if (const Segment * seg = i->cast<Segment>()) {
            TLOG_DEBUG(6) << "Disabling children of segment " << seg->UID();
            system.m_disabled_components.disable_children(*seg);
          }
        }
      }

      for (unsigned long count = 1; true; ++count) {
        const unsigned long num(system.m_disabled_components.size());

        TLOG_DEBUG(6) <<  "before auto-disabling iteration " << count << " the number of disabled components is " << num ;

        TLOG_DEBUG(6) <<  "System has " << rs_or.size() << " resourceSetORs";
        for (const auto& i : rs_or) {
          if (system.m_disabled_components.is_enabled(i)) {
            // check ANY child is disabled
            TLOG_DEBUG(6) << "ResourceSetOR " << i->UID() << " contains " << i->get_contains().size() << " resources";
            for (auto & i2 : i->get_contains()) {
              if (!system.m_disabled_components.is_enabled(i2)) {
                TLOG_DEBUG(6) <<  "disable resource-set-OR " << i->UID() << " because it's child " << i2 << " is disabled" ;
                system.m_disabled_components.disable(*i);
                system.m_disabled_components.disable_children(*i);
                break;
              }
            }
          }
        }

        TLOG_DEBUG(6) <<  "System has " << rs_and.size() << " resourceSetANDs";
        for (const auto& j : rs_and) {
          if (system.m_disabled_components.is_enabled(j)) {
            const std::vector<const ResourceBase*> &resources = j->get_contains();
            TLOG_DEBUG(6) << "Checking " << resources.size() << " ResourceSetAND resources";
            if (!resources.empty()) {
              // check ANY child is enabled
              bool found_enabled = false;
              for (auto & j2 : resources) {
                if (system.m_disabled_components.is_enabled(j2)) {
                  found_enabled = true;
                  TLOG_DEBUG(6) << "Found enabled resource " << j2->UID();
                  break;
                }
              }
              if (found_enabled == false) {
                TLOG_DEBUG(6) <<  "disable resource-set-AND " << j->UID() << " because all it's children are disabled" ;
                system.m_disabled_components.disable(*j);
                system.m_disabled_components.disable_children(*j);
              }
            }
          }
        }

        if (system.m_disabled_components.size() == num) {
          TLOG_DEBUG(6) <<  "after " << count << " iteration(s) auto-disabling algorithm found no newly disabled sets, exiting loop ..." ;
          break;
        }

        unsigned int iLimit(1000);
        if (count > iLimit) {
          ers::error(ReadMaxAllowedIterations(ERS_HERE, iLimit));
          break;
        }
      }
    }
  }

  bool result(!system.m_disabled_components.is_enabled(this));
  TLOG_DEBUG( 6) <<  "disabled(" << this << ")  (UID=" << UID() << ") returns " << std::boolalpha << result  ;
  return result;
}

unsigned long
DisabledComponents::get_num_of_slr_resources(const System& system)
{
  return (system.m_disabled_components.m_num_of_slr_enabled_resources + system.m_disabled_components.m_num_of_slr_disabled_resources);
}
