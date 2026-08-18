#include "confmodel/Application.hpp"
#include "confmodel/Resource.hpp"
#include "confmodel/ExcludableEntitySet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include "confmodel/confmodelIssues.hpp"
#include "confmodel/DisabledResources.hpp"

#include "logging/Logging.hpp"

#include "confmodel/test_circular_dependency.hpp"

using namespace dunedaq::confmodel;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


DisabledResources::DisabledResources(const ExcludableEntitySet* root,
                                     std::vector<const Resource*> initial_list)
{
  TLOG_DEBUG(2) <<  "construct the object from Resource " << root->UID() ;
  update(root, initial_list);
}


void DisabledResources::update(const ExcludableEntitySet* root,
                               std::vector<const Resource*> initial_list) {

  m_disabled.clear();

  m_initialised = true;

  // get list of all root's resource-sets also test any
  // circular dependencies between segments and resource sets
  TestCircularDependency cd_fuse("component \'is-disabled\' status", root);
  std::vector<const ExcludableEntitySet*> resource_sets;
  std::set<const Resource*> simple_resources;
  fill(*root, resource_sets, simple_resources, cd_fuse);

  for (auto & comp : initial_list) {
    disable(*comp);
    TLOG_DEBUG(6) << comp->UID() << " is disabled in session";
    if (const ExcludableEntitySet * rs = comp->cast<ExcludableEntitySet>()) {
      disable_children(*rs);
    }
  }

  for (unsigned long count = 1; true; ++count) {
    const unsigned long num(size()); // Remember current size

    TLOG_DEBUG(6) <<  "before auto-disabling iteration " << count << " the number of disabled components is " << num ;
    for (const auto& res_set : resource_sets) {
      if (is_enabled(res_set)) {
        if (res_set->compute_disabled_state(m_disabled)) {
          TLOG_DEBUG(6) <<  "disable custom resource-set- " << res_set->UID() << " because children are disabled" ;
          disable(*res_set);
          disable_children(*res_set);
        }
      }
    }

    for (auto& res : simple_resources) {
      if (is_enabled(res) && res->compute_disabled_state(m_disabled)) {
        TLOG_DEBUG(6) << "Marking " << res->UID() << " as disabled\n";
        disable(*res);
      }
    }

    if (size() == num) {
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

// fill data from resource sets
void DisabledResources::fill(const ExcludableEntitySet& rs,
                             std::vector<const ExcludableEntitySet*>& all_resource_sets,
                             std::set<const Resource*>& simple_resources,
                             TestCircularDependency& cd_fuse)
{
  TLOG_DEBUG(6) << "rs.UID=" << rs.UID() << ", class=" << rs.class_name();
  all_resource_sets.push_back(&rs);
  auto rptr = &rs;
  if (rptr->cast<Resource>() == nullptr) {
    throw (MissingConstructor(ERS_HERE, "Resource", rs.full_name()));
  }
  for (auto & res : rs.contained_resources()) {
    AddTestOnCircularDependency add_fuse_test(cd_fuse, res);
    if (const ExcludableEntitySet * rs2 = res->cast<ExcludableEntitySet>()) {
      fill(*rs2, all_resource_sets, simple_resources, cd_fuse);
    } else {
        simple_resources.insert(res);
    }
  }
}



void
DisabledResources::disable_children(const ExcludableEntitySet& rs)
{
  TLOG_DEBUG(6) << "Disabling children of " << rs.UID();
  for (auto & res : rs.contained_resources()) {
    TLOG_DEBUG(6) << "Disabling child " << res->UID();
    disable(*res);
    if (const auto * rs2 = res->cast<ExcludableEntitySet>()) {
      disable_children(*rs2);
    }
  }
}

