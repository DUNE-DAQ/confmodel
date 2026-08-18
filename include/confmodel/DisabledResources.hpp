#ifndef DUNEDAQDAL_DISABLED_RESOURCES_H
#define DUNEDAQDAL_DISABLED_RESOURCES_H

#include "confmodel/Resource.hpp"

#include <set>
#include <string>


namespace dunedaq::confmodel {

    class Session;
    class ExcludableEntitySet;
    class TestCircularDependency;

    class DisabledResources 
    {

      friend class Session;
      friend class Resource;

    private:

      std::set<std::string> m_disabled;
      bool m_initialised{false};
      void fill(const ExcludableEntitySet& rs,
                std::vector<const ExcludableEntitySet*>& all_resource_sets,
                std::set<const Resource*>& simple_resources,
                TestCircularDependency& cd_fuse);

      void
      disable(const Resource& component)
      {
        m_disabled.insert(component.UID());
      }

      void
      disable_children(const ExcludableEntitySet&);

      size_t
      size() noexcept
      {
        return m_disabled.size();
      }

    public:

      DisabledResources() = default;
      DisabledResources(const ExcludableEntitySet* root,
                        std::vector<const Resource*> initial_list);

      ~DisabledResources() = default;

      void update(const ExcludableEntitySet* root,
                  std::vector<const Resource*> initial_list);

      bool
      is_enabled(const Resource* component) const {
        return !m_disabled.contains(component->UID());
      }

      [[nodiscard]] bool initialised() const {return m_initialised;}
    };
} // namespace dunedaq::confmodel

#endif // DUNEDAQDAL_DISABLED_RESOURCES_H
