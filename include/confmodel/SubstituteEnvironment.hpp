#ifndef SUBSTITUTEENVIRONMENT_HPP
#define SUBSTITUTEENVIRONMENT_HPP

#include "conffwk/Configuration.hpp"
#include "conffwk/DalObject.hpp"
#include "confmodel/Segment.hpp"
#include "confmodel/Session.hpp"

#include <map>
#include <string>
#include <vector>

namespace dunedaq::confmodel {

    /**
     *  \brief Implements string converter for database parameters.
     *
     *  The class implements dunedaq::conffwk::Configuration::AttributeConverter for string %type.
     *  It reads parameters defined for given partition object and uses them to
     *  substitute values of database string attributes.
     *
     *  The parameters are stored as a map of substitution keys and values.
     *  If database %is changed, the reset(dunedaq::conffwk::Configuration&, const Session&) method needs to be used.
     *
     *  \par Example
     *
     *  The example shows how to use the converter:
     *
     *  <pre><i>
     *
     *  dunedaq::conffwk::Configuration db(...);  // some code to build configuration database object
     *
     *  const dunedaq::dal::Session * partition = dunedaq::dal::get_partition(db, partition_name);
     *  if(partition) {
     *    db.register_converter(new dunedaq::dal::SubstituteEnvironment(db, *partition));
     *  }
     *
     *  </i></pre>
     *
     */

  class SubstituteEnvironment : 
    public dunedaq::conffwk::Configuration::AttributeConverter<std::string> {
    
  public:
    SubstituteEnvironment() = default;

    /** Implementation of convert method. **/
    virtual void convert(std::string& value,
                         const dunedaq::conffwk::Configuration& conf,
                         const dunedaq::conffwk::ConfigObject& obj,
                         const std::string& attr_name) override;

    /** Destroy conversion map. **/
    virtual ~SubstituteEnvironment() {;}
  };

} // namespace

#endif // SUBSTITUTEENVIRONMENT_HPP
