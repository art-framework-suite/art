#ifndef art_Persistency_Provenance_ProcessConfiguration_h
#define art_Persistency_Provenance_ProcessConfiguration_h

// ======================================================================
//
// ProcessConfiguration
//
// ======================================================================

#include "canvas/Persistency/Provenance/PassID.h"
#include "canvas/Persistency/Provenance/ProcessConfigurationID.h"
#include "canvas/Persistency/Provenance/ReleaseVersion.h"
#include "fhiclcpp/ParameterSetID.h"
#include <iosfwd>
#include <string>

// ----------------------------------------------------------------------

namespace art {

  struct ProcessConfiguration {
    ProcessConfiguration() : processName_(), parameterSetID_(), releaseVersion_(), passID_() {}
    ProcessConfiguration(std::string const& procName,
                         fhicl::ParameterSetID const& pSetID,
                         ReleaseVersion const& relVersion,
                         PassID const& pass) :
      processName_(procName),
      parameterSetID_(pSetID),
      releaseVersion_(relVersion),
      passID_(pass) { }

    std::string const& processName() const {return processName_;}
    fhicl::ParameterSetID const& parameterSetID() const {return parameterSetID_;}
    ReleaseVersion const& releaseVersion() const {return releaseVersion_;}
    PassID const& passID() const {return passID_;}
    ProcessConfigurationID id() const;

    std::string processName_;
    fhicl::ParameterSetID parameterSetID_;
    ReleaseVersion releaseVersion_;
    PassID passID_;
  };

  bool
  operator<(ProcessConfiguration const& a, ProcessConfiguration const& b);

  inline
  bool
  operator==(ProcessConfiguration const& a, ProcessConfiguration const& b) {
    return a.processName() == b.processName() &&
    a.parameterSetID() == b.parameterSetID() &&
    a.releaseVersion() == b.releaseVersion() &&
    a.passID() == b.passID();
  }

  inline
  bool
  operator!=(ProcessConfiguration const& a, ProcessConfiguration const& b) {
    return !(a == b);
  }

  std::ostream&
  operator<< (std::ostream& os, ProcessConfiguration const& pc);

}  // art

// ======================================================================

#endif /* art_Persistency_Provenance_ProcessConfiguration_h */

// Local Variables:
// mode: c++
// End:
