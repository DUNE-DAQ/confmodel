#include "logging/Logging.hpp"

#include "conffwk/Configuration.hpp"

#include "confmodel/Component.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/System.hpp"

#include <iostream>
#include <string>

using namespace dunedaq;

void listApps(const confmodel::System* system) {
  for (auto app : system->get_all_applications()) {
    std::cout << "Application: " << app->UID();
    auto res = app->cast<confmodel::ResourceSet>();
    if (res) {
      if (res->disabled(*system)) {
        std::cout << "<disabled>";
      }
      else {
        for (auto mod : res->get_contains()) {
          std::cout << " " << mod->UID();
          if (mod->disabled(*system)) {
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

int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " system database-file\n";
    return 0;
  }

  dunedaq::logging::Logging::setup();

  std::string confimpl = "oksconflibs:" + std::string(argv[2]);
  auto confdb = new conffwk::Configuration(confimpl);

  std::string systemName(argv[1]);
  auto system = confdb->get<confmodel::System>(systemName);
  if (system == nullptr) {
    std::cerr << "System " << systemName << " not found in database\n";
    return -1;
  }


  std::cout << "Checking segments disabled state\n";
  auto rseg = system->get_segment();
  if (!rseg->disabled(*system)) {
    std::cout << "Root segment " << rseg->UID()
              << " is not disabled, looping over contained segments\n";
    for (auto seg : rseg->get_segments()) {
      std::cout << "Segment " << seg->UID()
                << std::string(seg->disabled(*system)? " is ":" is not ")
                << "disabled\n";
    }
  }

  auto disabled = system->get_disabled();
  std::cout << "======\nCurrently " << disabled.size() << " items disabled: ";
  for (auto item : disabled) {
    std::cout << " " << item->UID();
  }
  std::cout << std::endl;
  listApps(system);

  std::cout << "======\nNow trying to set enabled  \n";
  std::set<const confmodel::Component*> enable;
  for (auto item : disabled) {
    enable.insert(item);
  }
  system->set_enabled(enable);
  listApps(system);

  std::cout << "======\nNow trying to set enabled to an empty list\n";
  enable.clear();
  system->set_enabled(enable);
  listApps(system);

  std::cout << "======\nNow trying to set disabled to an empty list \n";
  system->set_disabled({});
  listApps(system);

}
