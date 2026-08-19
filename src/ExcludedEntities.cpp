#include "confmodel/Application.hpp"
#include "confmodel/Resource.hpp"
#include "confmodel/ExcludableEntitySet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include "confmodel/confmodelIssues.hpp"
#include "confmodel/ExcludedEntities.hpp"

#include "logging/Logging.hpp"

#include "confmodel/test_circular_dependency.hpp"

using namespace dunedaq::confmodel;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


ExcludedEntities::ExcludedEntities(const ExcludableEntitySet* root,
                                     std::vector<const Resource*> initial_list)
{
  TLOG_DEBUG(2) <<  "construct the object from Resource " << root->UID() ;
  update(root, initial_list);
}


void ExcludedEntities::update(const ExcludableEntitySet* root,
                               std::vector<const Resource*> initial_list) {

  m_excluded.clear();

  m_initialised = true;

  // get list of all root's entities-sets also test any
  // circular dependencies between segments and entity sets
  TestCircularDependency cd_fuse("component \'is-excluded\' status", root);
  std::vector<const ExcludableEntitySet*> resource_sets;
  std::set<const Resource*> simple_resources;
  fill(*root, resource_sets, simple_resources, cd_fuse);

  for (auto & comp : initial_list) {
    exclude(*comp);
    TLOG_DEBUG(6) << comp->UID() << " is excluded in session";
    if (const ExcludableEntitySet * rs = comp->cast<ExcludableEntitySet>()) {
      exclude_children(*rs);
    }
  }

  for (unsigned long count = 1; true; ++count) {
    const unsigned long num(size()); // Remember current size

    TLOG_DEBUG(6) <<  "before auto-disabling iteration " << count << " the number of excluded components is " << num ;
    for (const auto& res_set : resource_sets) {
      if (is_excluded(res_set)) {
        if (res_set->compute_excluded_state(m_excluded)) {
          TLOG_DEBUG(6) <<  "Exclude custom entity-set- " << res_set->UID() << " because children are excluded" ;
          exclude(*res_set);
          exclude_children(*res_set);
        }
      }
    }

    for (auto& res : simple_resources) {
      if (is_excluded(res) && res->compute_excluded_state(m_excluded)) {
        TLOG_DEBUG(6) << "Marking " << res->UID() << " as excluded\n";
        exclude(*res);
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

// fill data from entities sets
void ExcludedEntities::fill(const ExcludableEntitySet& rs,
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
  for (auto & res : rs.contained_excludable_entities()) {
    AddTestOnCircularDependency add_fuse_test(cd_fuse, res);
    if (const ExcludableEntitySet * rs2 = res->cast<ExcludableEntitySet>()) {
      fill(*rs2, all_resource_sets, simple_resources, cd_fuse);
    } else {
        simple_resources.insert(res);
    }
  }
}



void
ExcludedEntities::exclude_children(const ExcludableEntitySet& rs)
{
  TLOG_DEBUG(6) << "Excluding children of " << rs.UID();
  for (auto & res : rs.contained_excludable_entities()) {
    TLOG_DEBUG(6) << "Excluding child " << res->UID();
    exclude(*res);
    if (const auto * rs2 = res->cast<ExcludableEntitySet>()) {
      exclude_children(*rs2);
    }
  }
}

