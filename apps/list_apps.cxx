#include "logging/Logging.hpp"

#include "conffwk/Configuration.hpp"

#include "confmodel/Component.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include <iostream>
//#include <set>
#include <string>

using namespace dunedaq;


void process_segment(const confmodel::Session* session,
                     const confmodel::Segment* segment,
                     const std::set<std::string>& disabled_objects,
                     std::string spacer) {
  std::cout << spacer << "Segment " << segment->UID();
  bool segment_disabled = segment->disabled(*session);
  std::string reason = "";
  if (segment_disabled) {
    std::cout << " disabled";
    reason = "segment";
  }
  std::cout << "\n";
  for (auto subseg : segment->get_segments()) {
    process_segment (session, subseg, disabled_objects, spacer+"  ");
  }

  for (auto app : segment->get_applications()) {
    bool disabled = segment_disabled;
    std::cout << spacer << "  Application: " << app->UID();
    if (!disabled) {
      auto rset = app->cast<confmodel::ResourceSet>();
      if (rset) {
        if (rset->disabled(*session)) {
          disabled = true;
          if (disabled_objects.find(app->UID()) != disabled_objects.end()) {
            reason = "directly";
          }
          else {
            reason = "due to state of related objects";
          }
        }
        std::cout << " contains: {";
        std::string seperator = "";
        for (auto mod : rset->get_contains()) {
          std::cout << seperator << mod->UID();
          if (mod->disabled(*session)) {
            std::cout << "<disabled ";
            if (disabled_objects.find(mod->UID()) == disabled_objects.end()) {
              std::cout << "in";
            }
            std::cout << "directly>";
          }
          seperator = ", ";
        }
        std::cout << "}";
      }
    }
    if (disabled) {
      std::cout << " <disabled "<< reason << ">";
    }
    auto daqApp = app->cast<confmodel::DaqApplication>();
    if (daqApp) {
      std::cout << " Modules:";
      for (auto mod : daqApp->get_modules()) {
        std::cout << " " << mod->UID();
      }
    }

    std::cout << std::endl;
  }
}

int main(int argc, char* argv[]) {
  dunedaq::logging::Logging::setup();

  if (argc < 2) {
    std::cout << "Usage: " << argv[0] << " [session] database-file\n";
    return 0;
  }

  int filearg = 1;
  if (argc == 3) {
    filearg = 2;
  }
  std::string confimpl = "oksconflibs:" + std::string(argv[filearg]);
  auto confdb = new conffwk::Configuration(confimpl);

  std::string sessionName;
  if (argc == 3) {
    sessionName = std::string(argv[1]);
  }
  else {
    std::vector<conffwk::ConfigObject> session_obj;
    confdb->get("Session", session_obj);
    if (session_obj.size() == 0) {
      std::cerr << "Can't find any Sessions in database\n";
      return -1;
    }
    sessionName = session_obj[0].UID();
  }
  const confmodel::Session* session;
  session = confdb->get<confmodel::Session>(sessionName);
  if (session==nullptr) {
    std::cerr << "Session " << sessionName << " not found in database\n";
    return -1;
  }

  std::cout << "Applications in Session: " << sessionName << "\n";
  std::set<std::string> disabled_objects;
  for (auto object : session->get_disabled()) {
    TLOG_DEBUG(11) << object->UID() << " is in disabled list of Session";
    disabled_objects.insert(object->UID());
  }

  process_segment (session, session->get_segment(), disabled_objects, "");
}
