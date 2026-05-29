/**
 * @file list_apps.cpp
 *
 * This is part of the DUNE DAQ Software Suite, copyright 2020.
 * Licensing/copyright details are in the COPYING file that you should have
 * received with this code.
 */

#include "logging/Logging.hpp"

#include "conffwk/Configuration.hpp"

#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace dunedaq;
using namespace dunedaq::confmodel;


void process_segment(const Session* session,
                     const Segment* segment,
                     const std::set<std::string>& disabled_objects,
                     const std::string& spacer) {
  std::cout << spacer << "Segment " << segment->UID(); // NOLINT
  bool segment_disabled = segment->is_disabled(*session);
  std::string reason = "";
  if (segment_disabled) {
    std::cout << " disabled"; // NOLINT
    reason = "segment";
  }
  std::cout << "\n"; // NOLINT
  for (auto subseg : segment->get_segments()) {
    process_segment (session, subseg, disabled_objects, spacer+"  ");
  }

  for (auto app : segment->get_applications()) {
    bool disabled = segment_disabled;
    std::cout << spacer << "  Application: " << app->UID(); // NOLINT
    if (!disabled) {
      auto rset = app->cast<ResourceSet>();
      if (rset) {
        if (rset->is_disabled(*session)) {
          disabled = true;
          if (disabled_objects.find(app->UID()) != disabled_objects.end()) {
            reason = "directly";
          } else {
            reason = "due to state of related objects";
          }
        }
        std::cout << " contains: {"; // NOLINT
        std::string seperator = "";
        for (auto mod : rset->contained_resources()) {
          std::cout << seperator << mod->UID(); // NOLINT
          if (mod->is_disabled(*session)) {
            std::cout << "<disabled "; // NOLINT
            if (disabled_objects.find(mod->UID()) == disabled_objects.end()) {
              std::cout << "in"; // NOLINT
            }
            std::cout << "directly>"; // NOLINT
          }
          seperator = ", ";
        }
        std::cout << "}"; // NOLINT
      }
    }
    if (disabled) {
      std::cout << " <disabled "<< reason << ">"; // NOLINT
    }
    auto daqApp = app->cast<DaqApplication>();
    if (daqApp) {
      std::cout << " Modules:"; // NOLINT
      for (auto mod : daqApp->get_modules()) {
        std::cout << " " << mod->UID(); // NOLINT
      }
    }

    std::cout << std::endl; // NOLINT
  }
}

int main(int argc, char* argv[]) {

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [session] database-file\n"; // NOLINT
    return 0;
  }

  int filearg = 1;
  if (argc == 3) {
    filearg = 2;
  }
  std::string confimpl = "oksconflibs:" + std::string(
    argv[filearg]); //NOLINT
  conffwk::Configuration confdb(confimpl);

  std::vector<std::string> sessionList;
  if (argc == 3) {
    sessionList.emplace_back(
      argv[1]); //NOLINT
  } else {
    std::vector<conffwk::ConfigObject> session_obj;
    confdb.get("Session", session_obj);
    if (session_obj.size() == 0) {
      std::cerr << "Can't find any Sessions in database\n"; // NOLINT
      return -1;
    }
    for (const auto& obj : session_obj) {
      sessionList.push_back(obj.UID());
    }
  }
  dunedaq::logging::Logging::setup(sessionList[0], "list-apps"
  );

  std::string separator{};
  for (const auto& sessionName : sessionList) {
    const Session* session = confdb.get<Session>(sessionName);
    if (session==nullptr) {
      std::cerr << "Session " << sessionName << " not found in database\n"; // NOLINT
      return -1;
    }

    std::cout << separator << "      Applications in Session: " // NOLINT
              << sessionName << "\n"; // NOLINT
    std::set<std::string> disabled_objects;
    for (const auto& object : session->get_disabled()) {
      TLOG_DEBUG(11) << object->UID() << " is in disabled list of Session";
      disabled_objects.insert(object->UID());
    }

    process_segment (session, session->get_segment(),
                     disabled_objects,
                     "");
    separator =
      "\n   ----------------------------------------------\n\n";
  }
}

