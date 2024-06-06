#ifndef DUNEDAQDAL_DISABLED_COMPONENTS_H
#define DUNEDAQDAL_DISABLED_COMPONENTS_H

#include <string>
#include <vector>

#include "conffwk/Configuration.hpp"
#include "conffwk/ConfigAction.hpp"

#include "confmodel/Component.hpp"

namespace dunedaq::confmodel {

    class Session;
    class ResourceSet;
    // class Segment;

    class DisabledComponents : public dunedaq::conffwk::ConfigAction
    {

      friend class Session;
      friend class Component;

    private:

      struct SortStringPtr
      {
        bool
        operator()(const std::string * s1, const std::string * s2) const
        {
          return (*s1 < *s2);
        }
      };

      dunedaq::conffwk::Configuration& m_db;
      Session* m_session;

      unsigned long m_num_of_slr_enabled_resources;
      unsigned long m_num_of_slr_disabled_resources;

      std::set<const std::string *, SortStringPtr> m_disabled;
      std::set<const dunedaq::confmodel::Component *> m_user_disabled;
      std::set<const dunedaq::confmodel::Component *> m_user_enabled;

      void
      __clear() noexcept
      {
        m_disabled.clear();
        m_user_disabled.clear();
        m_user_enabled.clear();
        m_num_of_slr_enabled_resources = 0;
        m_num_of_slr_disabled_resources = 0;
      }

    public:

      DisabledComponents(dunedaq::conffwk::Configuration& db, Session* session);

      virtual
      ~DisabledComponents();

      void
      notify(std::vector<dunedaq::conffwk::ConfigurationChange *>& /*changes*/) noexcept;

      void
      load() noexcept;

      void
      unload() noexcept;

      void
      update(const dunedaq::conffwk::ConfigObject& obj, const std::string& name) noexcept;

      void
      reset() noexcept;

      size_t
      size() noexcept
      {
        return m_disabled.size();
      }

      void
      disable(const dunedaq::confmodel::Component& c)
      {
        m_disabled.insert(&c.UID());
      }

      bool
      is_enabled(const dunedaq::confmodel::Component* c) {
        return (m_disabled.find(&c->UID()) == m_disabled.end());
      }

      void
      disable_children(const dunedaq::confmodel::ResourceSet&);

      void
      disable_children(const dunedaq::confmodel::Segment&);

      static unsigned long
      get_num_of_slr_resources(const dunedaq::confmodel::Session& p);

    };
} // namespace dunedaq::confmodel

#endif // DUNEDAQDAL_DISABLED_COMPONENTS_H
