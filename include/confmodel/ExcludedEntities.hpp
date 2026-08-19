#ifndef DUNEDAQDAL_EXCLUDED_ENTITIES_H
#define DUNEDAQDAL_EXCLUDED_ENTITIES_H

#include "confmodel/Resource.hpp"

#include <set>
#include <string>


namespace dunedaq::confmodel {

    class Session;
    class ExcludableEntitySet;
    class TestCircularDependency;

    class ExcludedEntities 
    {

      friend class Session;
      friend class Resource;

    private:

      std::set<std::string> m_excluded;
      bool m_initialised{false};
      void fill(const ExcludableEntitySet& rs,
                std::vector<const ExcludableEntitySet*>& all_resource_sets,
                std::set<const Resource*>& simple_resources,
                TestCircularDependency& cd_fuse);

      void
      exclude(const Resource& component)
      {
        m_excluded.insert(component.UID());
      }

      void
      exclude_children(const ExcludableEntitySet&);

      size_t
      size() noexcept
      {
        return m_excluded.size();
      }

    public:

      ExcludedEntities() = default;
      ExcludedEntities(const ExcludableEntitySet* root,
                        std::vector<const Resource*> initial_list);

      ~ExcludedEntities() = default;

      void update(const ExcludableEntitySet* root,
                  std::vector<const Resource*> initial_list);

      bool
      is_included(const Resource* component) const {
        return ! m_excluded.contains(component->UID());
      }

      [[nodiscard]] bool initialised() const {return m_initialised;}
    };
} // namespace dunedaq::confmodel

#endif // DUNEDAQDAL_EXCLUDED_ENTITIES_H
