#include "logging/Logging.hpp"

#include "conffwk/Configuration.hpp"

#include "confmodel/Component.hpp"
#include "confmodel/DetectorConfig.hpp"
#include "confmodel/VariableBase.hpp"
#include "confmodel/Variable.hpp"
#include "confmodel/VariableSet.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"
#include "confmodel/SubstituteEnvironment.hpp"

#include <iostream>
#include <string>

using namespace dunedaq;
int main(int argc, char* argv[]) {
  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " session database-file\n";
    return 0;
  }

  dunedaq::logging::Logging::setup();

  std::string confimpl = "oksconflibs:" + std::string(argv[2]);
  auto confdb = new conffwk::Configuration(confimpl);

  std::string sessionName(argv[1]);
  auto session = confdb->get<confmodel::Session>(sessionName);
  if (session == nullptr) {
    std::cerr << "Session " << sessionName << " not found in database\n";
    return -1;
  }

  confmodel::SubstituteEnvironment converter;
  confdb->register_converter<std::string>(&converter);

  for (auto base_var : session->get_environment()) {
    if (const auto var = base_var->cast<confmodel::Variable>()) {
      std::cout << var->UID()
                << ": name=" << var->get_name()
                << ", value=" << var->get_value() << "\n";
    }
  }
}
