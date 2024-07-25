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

  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " session database-file\n";
    return 0;
  }
  std::string confimpl = "oksconflibs:" + std::string(argv[2]);
  auto confdb = new conffwk::Configuration(confimpl);

  std::string sessionName(argv[1]);
  auto session = confdb->get<confmodel::Session>(sessionName);
  if (session==nullptr) {
    std::cerr << "Session " << sessionName << " not found in database\n";
    return -1;
  }

  std::set<std::string> disabled_objects;
  for (auto object : session->get_disabled()) {
    TLOG_DEBUG(11) << object->UID() << " is in disabled list of Session";
    disabled_objects.insert(object->UID());
  }

  process_segment (session, session->get_segment(), disabled_objects, "");
}
