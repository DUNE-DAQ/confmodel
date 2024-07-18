#include "logging/Logging.hpp"

#include "conffwk/Configuration.hpp"

#include "confmodel/Component.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include <iostream>
#include <string>

using namespace dunedaq;


void process_segment(const confmodel::Session* session,
                     const confmodel::Segment* segment) {
  bool disabled = segment->disabled(*session);
  for (auto subseg : segment->get_segments()) {
    process_segment (session, subseg);
  }
  for (auto app : segment->get_applications()) {
    std::cout << "Application: " << app->UID();
    if (disabled) {
      std::cout << "<disabled segment>";
    }
    else {
      auto res = app->cast<confmodel::ResourceSet>();
      if (res) {
        if (res->disabled(*session)) {
          std::cout << "<disabled>";
        }
        else {
          for (auto mod : res->get_contains()) {
            std::cout << " " << mod->UID();
            if (mod->disabled(*session)) {
              std::cout << "<disabled>";
            }
          }
        }
      }
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

  process_segment (session, session->get_segment());
}
