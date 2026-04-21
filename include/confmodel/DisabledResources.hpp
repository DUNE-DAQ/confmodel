/**
 * @file DisabledResources.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef CONFMODEL_INCLUDE_CONFMODEL_DISABLEDRESOURCES_HPP_
#define CONFMODEL_INCLUDE_CONFMODEL_DISABLEDRESOURCES_HPP_

#include "confmodel/Resource.hpp"

#include <set>
#include <string>
#include <vector>

namespace dunedaq::confmodel {

    class Session;
    class ResourceSet;
    class TestCircularDependency;

    class DisabledResources 
    {

      friend class Session;
      friend class Resource;

    public:
      DisabledResources() = default;
      DisabledResources(const ResourceSet* root,
                        std::vector<const Resource*> initial_list);

      ~DisabledResources() = default;

      void update(const ResourceSet* root,
                  std::vector<const Resource*> initial_list);

      bool
      is_enabled(const Resource* component) const {
        return !m_disabled.contains(component->UID());
      }

      [[nodiscard]] bool initialised() const {return m_initialised;}


    private:

      std::set<std::string> m_disabled;
      bool m_initialised{false};
      void fill(const ResourceSet& rs,
                std::vector<const ResourceSet*>& all_resource_sets,
                TestCircularDependency& cd_fuse);

      void
      disable(const Resource& component)
      {
        m_disabled.insert(component.UID());
      }

      void
      disable_children(const ResourceSet&);

      size_t
      size() noexcept
      {
        return m_disabled.size();
      }

    };
} // namespace dunedaq::confmodel

#endif // CONFMODEL_INCLUDE_CONFMODEL_DISABLEDRESOURCES_HPP_
