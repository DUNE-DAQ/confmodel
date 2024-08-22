#ifndef SUBSTITUTEENVIRONMENT_HPP
#define SUBSTITUTEENVIRONMENT_HPP

#include "conffwk/Configuration.hpp"

#include <string>

namespace dunedaq::confmodel {

    /**
     *  \brief Implements string converter for database parameters.
     *
     *  The class implements
     *  dunedaq::conffwk::Configuration::AttributeConverter for string
     *  type.  It substitutes values of database string attributes
     *  with values from the environment.
     *
     *  If database is changed, the
     *  reset(dunedaq::conffwk::Configuration&, const Session&) method
     *  needs to be used.
     *
     *  \par Example
     *
     *  The example shows how to use the converter:
     *
     *  <pre><i>
     *
     *  dunedaq::conffwk::Configuration db(...);  // some code to build configuration database object
     *
     *  db.register_converter<std::string>(
     *                     new dunedaq::confmodel::SubstituteEnvironment());
     *
     *  </i></pre>
     *
     */

  class SubstituteEnvironment : 
    public dunedaq::conffwk::Configuration::AttributeConverter<std::string> {
    
  public:
    SubstituteEnvironment() = default;
    SubstituteEnvironment(const SubstituteEnvironment&) = delete;
    SubstituteEnvironment(const SubstituteEnvironment&&) = delete;
    SubstituteEnvironment& operator =(const SubstituteEnvironment&) = delete;
    SubstituteEnvironment& operator =(const SubstituteEnvironment&&) = delete;
    virtual ~SubstituteEnvironment() = default;

    /** Implementation of convert method. **/
    void convert(std::string& value,
                 const dunedaq::conffwk::Configuration& conf,
                 const dunedaq::conffwk::ConfigObject& obj,
                 const std::string& attr_name) override;

  };

} // namespace dunedaq::confmodel

#endif // SUBSTITUTEENVIRONMENT_HPP
