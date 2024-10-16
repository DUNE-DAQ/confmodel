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
  session_get_all_applications(const Configuration& db,
                               const std::string& session_name) {
    auto session=const_cast<Configuration&>(db).get<Session>(session_name);
    std::vector<ObjectLocator> apps;
    for (auto app : session->get_all_applications()) {
      apps.push_back({app->UID(),app->class_name()});
    }
    return apps;
  }

  std::vector<ObjectLocator>
  session_get_enabled_applications(const Configuration& db,
                                   const std::string& session_name) {
    auto session=const_cast<Configuration&>(db).get<Session>(session_name);
    std::vector<ObjectLocator> apps;
    for (auto app : session->get_enabled_applications()) {
      apps.push_back({app->UID(),app->class_name()});
    }
    return apps;
  }

  void
  session_set_disabled(const Configuration& db,
                       const std::string& session_name,
                       const std::vector<std::string>& comps) {
    auto session=const_cast<Configuration&>(db).get<Session>(session_name);
    std::set<const Component*> objs;
    for (auto comp: comps) {
      auto obj=const_cast<Configuration&>(db).get<Component>(comp);
      objs.insert(obj);
    }
    session->set_disabled(objs);
  }

  bool component_disabled(const Configuration& db, const std::string& session_id, const std::string& component_id) {
    try {
      ConfigObject object;
      const_cast<Configuration&>(db).get("Component", component_id, object);
    }
    catch (conffwk::NotFound& except) {
      return false;
    }
    const dunedaq::confmodel::Component* component_ptr = const_cast<Configuration&>(db).get<dunedaq::confmodel::Component>(component_id);
    const dunedaq::confmodel::Session* session_ptr = const_cast<Configuration&>(db).get<dunedaq::confmodel::Session>(session_id);

    return component_ptr->disabled(*session_ptr);
  }


  std::vector<std::vector<ObjectLocator>> component_get_parents(const Configuration& db,
                                                                const std::string& session_id,
                                                                const std::string& component_id) {
    const dunedaq::confmodel::Component* component_ptr = const_cast<Configuration&>(db).get<dunedaq::confmodel::Component>(component_id);
    const dunedaq::confmodel::Session* session_ptr = const_cast<Configuration&>(db).get<dunedaq::confmodel::Session>(session_id);

    std::list<std::vector<const dunedaq::confmodel::Component*>> parents;
    std::vector<std::vector<ObjectLocator>> parent_ids;

    component_ptr->get_parents(*session_ptr, parents);

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

  std::vector<std::string> daq_application_get_used_hostresources(const Configuration& db, const std::string& app_id) {
    auto app = const_cast<Configuration&>(db).get<dunedaq::confmodel::DaqApplication>(app_id);
    std::vector<std::string> resources;
    for (auto res : app->get_used_hostresources()) {
      resources.push_back(res->UID());
    }
    return resources;
  }

  std::vector<std::string> daq_application_construct_commandline_parameters(const Configuration& db,
                                                                            const std::string& session_id,
                                                                            const std::string& app_id) {
    const auto* app = const_cast<Configuration&>(db).get<dunedaq::confmodel::DaqApplication>(app_id);
    const auto* session = const_cast<Configuration&>(db).get<dunedaq::confmodel::Session>(session_id);
    return app->construct_commandline_parameters(db, session);
  }

  std::vector<std::string> rc_application_construct_commandline_parameters(const Configuration& db,
                                                                           const std::string& session_id,
                                                                           const std::string& app_id) {
    const auto* app = const_cast<Configuration&>(db).get<dunedaq::confmodel::RCApplication>(app_id);
    const auto* session = const_cast<Configuration&>(db).get<dunedaq::confmodel::Session>(session_id);
    return app->construct_commandline_parameters(db, session);
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
  m.def("session_set_disabled", &session_set_disabled, "Temporarily disable Components in the requested session");

  m.def("component_disabled", &component_disabled, "Determine if a Component-derived object (e.g. a Segment) has been disabled");
  m.def("component_get_parents", &component_get_parents, "Get the Component-derived class instances of the parent(s) of the Component-derived object in question");
  m.def("daqapp_get_used_resources", &daq_application_get_used_hostresources, "Get list of HostResources used by DAQApplication");
  m.def("daq_application_construct_commandline_parameters", &daq_application_construct_commandline_parameters, "Get a version of the command line agruments parsed");
  m.def("rc_application_construct_commandline_parameters", &rc_application_construct_commandline_parameters, "Get a version of the command line agruments parsed");
}

} // namespace dunedaq::confmodel::python
