#include "confmodel/ModuleFactory.hpp"
#include "confmodel/SmartDaqApplication.hpp"
#include "confmodel/util.hpp"

#include "oks/kernel.hpp"

using namespace dunedaq::confmodel;

std::vector<const dunedaq::confmodel::DaqModule*>
SmartDaqApplication::generate_modules(conffwk::Configuration* confdb,
                                      const std::string& dbfile,
                                      const confmodel::Session* session) const {
  oks::OksFile::set_nolock_mode(true);
  return ModuleFactory::instance().generate(class_name(),
                                            this,
                                            confdb,
                                            dbfile,
                                            session);
}

const std::vector<std::string> SmartDaqApplication::construct_commandline_parameters(
    const conffwk::Configuration& confdb,
    const dunedaq::confmodel::Session* session) const {
    return dunedaq::confmodel::construct_commandline_parameters_appfwk<SmartDaqApplication>(this, confdb, session);
}
