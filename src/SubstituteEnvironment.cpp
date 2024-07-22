#include "confmodel/SubstituteEnvironment.hpp"

namespace dunedaq::confmodel {


void SubstituteEnvironment::convert(std::string& value,
                                  const conffwk::Configuration&,
                                  const conffwk::ConfigObject& conf_obj,
                                  const std::string& attr) {
  TLOG_DEBUG(5) <<  "convert attribute \'" << attr << "\' value \'" << value << "\' of object " << &conf_obj ;

  static const std::string subs_begin("${");
  static const std::string subs_end("}");

  std::string::size_type pos = 0;       // position of tested string index
  std::string::size_type p_start = 0;   // beginning of variable
  std::string::size_type p_end = 0;     // beginning of variable

  auto str_from = value;
  while(
   ((p_start = value.find(subs_begin, pos)) != std::string::npos) &&
   ((p_end = value.find(subs_end, p_start + subs_begin.size())) != std::string::npos)) {
    std::string var(value, p_start + subs_begin.size(), p_end - p_start - subs_begin.size());

    if(char * env = getenv(var.c_str())) {
      value.replace(p_start, p_end - p_start + subs_end.size(), env);
    }
    else {
      std::ostringstream text;
      text << "substitution failed for parameter \'" << std::string(value, p_start, p_end - p_start + subs_end.size()) << '\'';
      throw dunedaq::conffwk::Generic(ERS_HERE, text.str().c_str());
    }
    pos = p_start + 1;
  }
}

} // namespace
