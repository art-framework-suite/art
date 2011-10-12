#ifndef art_Persistency_Provenance_ModuleDescription_h
#define art_Persistency_Provenance_ModuleDescription_h

/*----------------------------------------------------------------------

ModuleDescription: The description of a producer module.

----------------------------------------------------------------------*/

#include "art/Persistency/Provenance/ModuleDescriptionID.h"
#include "art/Persistency/Provenance/ProcessConfiguration.h"
#include "art/Persistency/Provenance/ProcessConfigurationID.h"
#include "fhiclcpp/ParameterSetID.h"
#include <iosfwd>
#include <string>

// ----------------------------------------------------------------------

namespace art {

  // once a module is born, these parts of the module's product provenance
  // are constant   (change to ModuleDescription)

  struct ModuleDescription {

    ModuleDescription();

    void write(std::ostream & os) const;

    fhicl::ParameterSetID const & parameterSetID() const {return parameterSetID_;}
    std::string const & moduleName() const {return moduleName_;}
    std::string const & moduleLabel() const {return moduleLabel_;}
    ProcessConfiguration const & processConfiguration() const {return processConfiguration_;}
    ProcessConfigurationID const processConfigurationID() const {return processConfiguration().id();}
    std::string const & processName() const {return processConfiguration().processName();}
    std::string const & releaseVersion() const {return processConfiguration().releaseVersion();}
    std::string const & passID() const {return processConfiguration().passID();}
    fhicl::ParameterSetID const & mainParameterSetID() const {return processConfiguration().parameterSetID();}

    // compiler-written copy c'tor, assignment, and d'tor are correct.

    bool operator<(ModuleDescription const & rh) const;

    bool operator==(ModuleDescription const & rh) const;

    bool operator!=(ModuleDescription const & rh) const;

    ModuleDescriptionID id() const; // For backward compatibility

    // ID of parameter set of the creator
    fhicl::ParameterSetID parameterSetID_;

    // The class name of the creator
    std::string moduleName_;

    // A human friendly string that uniquely identifies the EDProducer
    // and becomes part of the identity of a product that it produces
    std::string moduleLabel_;

    // The process configuration.
    ProcessConfiguration processConfiguration_;
  };

  inline
  std::ostream &
  operator<<(std::ostream & os, const ModuleDescription & p)
  {
    p.write(os);
    return os;
  }

} // art

// ======================================================================

#endif /* art_Persistency_Provenance_ModuleDescription_h */

// Local Variables:
// mode: c++
// End:
