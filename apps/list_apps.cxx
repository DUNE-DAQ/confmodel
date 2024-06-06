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
  for (auto app : session->get_all_applications()) {
    std::cout << "Application: " << app->UID();
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
