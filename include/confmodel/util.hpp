/**
 * @file util.hpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#ifndef CONFMODEL_INCLUDE_CONFMODEL_UTIL_HPP_
#define CONFMODEL_INCLUDE_CONFMODEL_UTIL_HPP_

#include "conffwk/Configuration.hpp"
// #include "conffwk/DalObject.hpp"
#include "nlohmann/json.hpp"
#include "logging/Logging.hpp" // NOTE: if ISSUES ARE DECLARED BEFORE include logging/Logging.hpp, TLOG_DEBUG<<issue wont work.

#include "confmodel/Application.hpp"
#include "confmodel/PhysicalHost.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Service.hpp"
#include "confmodel/Session.hpp"
#include "confmodel/VirtualHost.hpp"
#include "confmodel/confmodelIssues.hpp"

#include <exception>
#include <string>
#include <vector>

namespace dunedaq::confmodel {

template <typename T>
std::vector<std::string> construct_commandline_parameters_appfwk(
    const T *app, const conffwk::Configuration &confdb,
    const dunedaq::confmodel::Session *session) {

  const dunedaq::confmodel::Service *control_service = nullptr;

  for (auto const *as : app->get_exposes_service()) {
    if (as->UID().ends_with("_control")) {
      if (control_service)
        throw DuplicatedControlService(ERS_HERE, as->UID());
      control_service = as;
    }
  }

  if (control_service == nullptr)
    throw NoControlServiceDefined(ERS_HERE, app->UID());

  const std::string control_uri = control_service->get_protocol() + "://" +
                                  app->get_runs_on()->get_runs_on()->UID() +
                                  ":" +
                                  std::to_string(control_service->get_port());

  return {
      "-s",
      session->UID(),
      "--name",
      app->UID(),
      "-c",
      control_uri,
      "--configurationService",
      confdb.get_impl_spec(),
  };
}

} // namespace dunedaq::confmodel

#endif // CONFMODEL_INCLUDE_CONFMODEL_UTIL_HPP_
