#include "logging/Logging.hpp"

#include "conffwk/Configuration.hpp"

#include "confmodel/Component.hpp"
#include "confmodel/DaqApplication.hpp"
#include "confmodel/DaqModule.hpp"
#include "confmodel/ResourceSet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/System.hpp"

#include <iostream>
//#include <set>
#include <string>

using namespace dunedaq;


void process_segment(const confmodel::System* system,
                     const confmodel::Segment* segment,
                     const std::set<std::string>& disabled_objects,
                     std::string spacer) {
  std::cout << spacer << "Segment " << segment->UID();
  bool segment_disabled = segment->disabled(*system);
  std::string reason = "";
  if (segment_disabled) {
    std::cout << " disabled";
    reason = "segment";
  }
  std::cout << "\n";
  for (auto subseg : segment->get_segments()) {
    process_segment (system, subseg, disabled_objects, spacer+"  ");
  }

  for (auto app : segment->get_applications()) {
    bool disabled = segment_disabled;
    std::cout << spacer << "  Application: " << app->UID();
    if (!disabled) {
      auto rset = app->cast<confmodel::ResourceSet>();
      if (rset) {
        if (rset->disabled(*system)) {
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
          if (mod->disabled(*system)) {
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
    std::cout << "Usage: " << argv[0] << " [system] database-file\n";
    return 0;
  }

  int filearg = 1;
  if (argc == 3) {
    filearg = 2;
  }
  std::string confimpl = "oksconflibs:" + std::string(argv[filearg]);
  auto confdb = new conffwk::Configuration(confimpl);

  std::vector<std::string> systemList;
  if (argc == 3) {
    systemList.emplace_back(std::string(argv[1]));
  }
  else {
    std::vector<conffwk::ConfigObject> system_obj;
    confdb->get("System", system_obj);
    if (system_obj.size() == 0) {
      std::cerr << "Can't find any Systems in database\n";
      return -1;
    }
    for (auto obj : system_obj) {
      systemList.push_back(obj.UID());
    }
  }

  std::string separator{};
  for (const auto& systemName : systemList) {
    const confmodel::System* system;
    system = confdb->get<confmodel::System>(systemName);
    if (system==nullptr) {
      std::cerr << "System " << systemName << " not found in database\n";
      return -1;
    }

    std::cout << separator << "      Applications in System: "
              << systemName << "\n";
    std::set<std::string> disabled_objects;
    for (auto object : system->get_disabled()) {
      TLOG_DEBUG(11) << object->UID() << " is in disabled list of System";
      disabled_objects.insert(object->UID());
    }

    process_segment (system, system->get_segment(),
                     disabled_objects,
                     "");
    separator =
      "\n   ----------------------------------------------\n\n";
  }
}
