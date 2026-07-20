/**
 * @file dal_methods.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "pybind11/operators.h"
#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

#include "confmodel/Application.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DetectorToDaqConnection.hpp"
#include "confmodel/DetectorStream.hpp"
#include "confmodel/DetDataReceiver.hpp"
#include "confmodel/DetDataSender.hpp"
#include "confmodel/HostComponent.hpp"
#include "confmodel/RCApplication.hpp"
#include "confmodel/Session.hpp"


#include <sstream>

namespace py = pybind11;
using namespace dunedaq::conffwk;

namespace dunedaq::confmodel::python {

  struct ObjectLocator {
    ObjectLocator(const std::string& id_arg, const std::string& class_name_arg) :
      id(id_arg), class_name(class_name_arg)
      {}
    const std::string id;
    const std::string class_name;
  };


  std::vector<ObjectLocator>
  session_get_all_applications(Configuration& db,
                               const std::string& session_name) {
    auto session=db.get<Session>(session_name);
    std::vector<ObjectLocator> apps;
    for (auto app : session->all_applications()) {
      apps.push_back({app->UID(),app->class_name()});
    }
    return apps;
  }

  std::vector<ObjectLocator>
  session_get_enabled_applications(Configuration& db,
                                   const std::string& session_name) {
    auto session=db.get<Session>(session_name);
    std::vector<ObjectLocator> apps;
    for (auto app : session->enabled_applications()) {
      apps.push_back({app->UID(),app->class_name()});
    }
    return apps;
  }


  void disable_component(Configuration& db,
                         const std::string& session_id,
                         const std::string& component_id) {
    auto session_ptr = const_cast<dunedaq::confmodel::Session*>(db.get<dunedaq::confmodel::Session>(session_id));
    auto component_ptr = db.get<dunedaq::confmodel::Resource>(component_id);
    if (session_ptr == nullptr) {
      throw (std::runtime_error(std::format("Session {} not found", session_id)));
    }
    if (component_ptr == nullptr) {
      throw (std::runtime_error(std::format("Component {} not found", component_id)));
    }
    session_ptr->disable(component_ptr);
  }
  void enable_component(Configuration& db,
                         const std::string& session_id,
                         const std::string& component_id) {
    auto session_ptr = const_cast<dunedaq::confmodel::Session*>(db.get<dunedaq::confmodel::Session>(session_id));
    auto component_ptr = db.get<dunedaq::confmodel::Resource>(component_id);
    if (session_ptr == nullptr) {
      throw (std::runtime_error(std::format("Session {} not found", session_id)));
    }
    if (component_ptr == nullptr) {
      throw (std::runtime_error(std::format("Component {} not found", component_id)));
    }
    session_ptr->enable(component_ptr);
  }


  bool component_disabled(Configuration& db,
                          const std::string& session_id,
                          const std::string& component_id) {
    const dunedaq::confmodel::Session* session_ptr = db.get<dunedaq::confmodel::Session>(session_id);
    const dunedaq::confmodel::Resource* component_ptr = db.get<dunedaq::confmodel::Resource>(component_id);
    if (component_ptr == nullptr) {
      return false;
    }
    return component_ptr->is_disabled(*session_ptr);
  }

  std::string why_disabled(Configuration& db,
                          const std::string& session_id,
                          const std::string& component_id) {
    if (component_disabled(db, session_id, component_id)) {
      const dunedaq::confmodel::Resource* component_ptr =
        db.get<dunedaq::confmodel::Resource>(component_id);
      if (component_ptr == nullptr) {
        throw (std::runtime_error(std::format("Component {} not found", component_id)));
      }
      return component_ptr->why_disabled();
    } else {
      return "not disabled";
    }
  }

  std::vector<std::string>
  contained_components(Configuration& db,
                       const std::string& set_id) {
    std::vector<std::string> result;
    const dunedaq::confmodel::ResourceSet* set_ptr = db.get<dunedaq::confmodel::ResourceSet>(set_id);
    if (set_ptr == nullptr) {
      return result;
    }
    for (auto res : set_ptr->contained_resources()) {
      result.emplace_back(res->UID());
    }
    return result;
  }

  std::vector<std::vector<ObjectLocator>> component_get_parents(Configuration& db,
                                                                const std::string& session_id,
                                                                const std::string& component_id) {
    const dunedaq::confmodel::Session* session_ptr = db.get<dunedaq::confmodel::Session>(session_id);
    const dunedaq::confmodel::Resource* component_ptr = db.get<dunedaq::confmodel::Resource>(component_id);

    std::list<std::vector<const dunedaq::confmodel::Resource*>> parents;
    std::vector<std::vector<ObjectLocator>> parent_ids;

    component_ptr->parents(*session_ptr, parents);

    for (const auto& parent : parents) {
      std::vector<ObjectLocator> parents_components;
      for (const auto& ancestor_component_ptr : parent) {
        parents_components.emplace_back(
          ObjectLocator(ancestor_component_ptr->UID(),
                        ancestor_component_ptr->class_name()) );
      }
      parent_ids.emplace_back(parents_components);
    }
    return parent_ids;
  }

  std::vector<std::string> daq_application_get_used_hostresources(Configuration& db, const std::string& app_id) {
    auto app = db.get<dunedaq::confmodel::DaqApplication>(app_id);
    std::vector<std::string> resources;
    for (auto res : app->get_used_hostresources()) {
      resources.push_back(res->UID());
    }
    return resources;
  }

  std::vector<std::string> daq_application_construct_commandline_parameters(Configuration& db,
                                                                            const std::string& session_id,
                                                                            const std::string& app_id) {
    const auto* app = db.get<dunedaq::confmodel::DaqApplication>(app_id);
    const auto* session = db.get<dunedaq::confmodel::Session>(session_id);
    return app->construct_commandline_parameters(db, session);
  }

  std::vector<std::string> rc_application_construct_commandline_parameters(Configuration& db,
                                                                           const std::string& session_id,
                                                                           const std::string& app_id) {
    const auto* app = db.get<dunedaq::confmodel::RCApplication>(app_id);
    const auto* session = db.get<dunedaq::confmodel::Session>(session_id);
    return app->construct_commandline_parameters(db, session);
  }


  std::string d2d_receiver(Configuration& db,
                           const std::string& d2d_id) {
    const auto* d2d = db.get<dunedaq::confmodel::DetectorToDaqConnection>(d2d_id);
    if (d2d == nullptr) {
      return "";
    }
    return d2d->receiver()->UID();
  }

  std::vector<std::string> d2d_senders(Configuration& db,
                                       const std::string& d2d_id) {
    std::vector<std::string> senders;
    const auto* d2d = db.get<dunedaq::confmodel::DetectorToDaqConnection>(d2d_id);
    if (d2d != nullptr) {
      for (auto sender: d2d->senders()) {
        senders.push_back(sender->UID());
      }
    }
    return senders;
  }

  std::vector<std::string> d2d_streams(Configuration& db,
                                       const std::string& d2d_id) {
    std::vector<std::string> streams;
    const auto* d2d = db.get<dunedaq::confmodel::DetectorToDaqConnection>(d2d_id);
    if (d2d != nullptr) {
      for (auto stream: d2d->streams()) {
        streams.push_back(stream->UID());
      }
    }
    return streams;
  }

  std::vector<std::string>
  resourceset_contains(Configuration& db,
                       const std::string& res_id) {
    std::vector<std::string> resources;
    auto res_set = db.get<dunedaq::confmodel::ResourceSet>(res_id);
    if (res_set != nullptr) {
      for (auto res: res_set->contained_resources()) {
        resources.push_back(res->UID());
      }
    }
    return resources;
  }


void
register_dal_methods(py::module& m)
{
  py::class_<ObjectLocator>(m, "ObjectLocator")
    .def(py::init<const std::string&, const std::string&>())
    .def_readonly("id", &ObjectLocator::id)
    .def_readonly("class_name", &ObjectLocator::class_name)
    ;

  m.def("session_get_all_applications", &session_get_all_applications, "Get list of ALL applications (regardless of enabled/disabled state) in the requested session");
  m.def("session_get_enabled_applications", &session_get_enabled_applications, "Get list of enabled applications in the requested session");

  m.def("disable_component", &disable_component, "Disable a Resource-derived object (e.g. a Segment)");
  m.def("enable_component", &enable_component, "Enable a Resource-derived object (e.g. a Segment)");

  m.def("component_disabled", &component_disabled, "Determine if a Resource-derived object (e.g. a Segment) has been disabled");
  m.def("why_disabled", &why_disabled, "Why is a Resource-derived object (e.g. a Segment) disabled");
  m.def("contained_components", &contained_components, "Get list of components of  a ResourceSet object");

  m.def("component_get_parents", &component_get_parents, "Get the Resource-derived class instances of the parent(s) of the Resource-derived object in question");
  m.def("daqapp_get_used_resources", &daq_application_get_used_hostresources, "Get list of HostResources used by DAQApplication");
  m.def("daq_application_construct_commandline_parameters", &daq_application_construct_commandline_parameters, "Get a version of the command line agruments parsed");
  m.def("rc_application_construct_commandline_parameters", &rc_application_construct_commandline_parameters, "Get a version of the command line agruments parsed");

  m.def("d2d_receiver", &d2d_receiver, "Get receiver associated with DetectorToDaqConnection");
  m.def("d2d_senders", &d2d_senders, "Get senders associated with DetectorToDaqConnection");
  m.def("d2d_streams", &d2d_streams, "Get streams associated with DetectorToDaqConnection");
  m.def("resourceset_contains", &resourceset_contains, "Get contained Resources from ResourceSet");
}

} // namespace dunedaq::confmodel::python
