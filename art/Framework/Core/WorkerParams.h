
#ifndef art_Framework_Core_WorkerParams_h
#define art_Framework_Core_WorkerParams_h

// This struct is used to communicate parameters into the worker factory.

#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "art/Persistency/Provenance/PassID.h"
#include "art/Persistency/Provenance/ReleaseVersion.h"
#include "art/Utilities/GetPassID.h"
#include "art/Version/GetReleaseVersion.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

namespace art {
  class ActionTable;

  struct WorkerParams
  {
    WorkerParams():
      procPset_(0), pset_(0),reg_(0),actions_(0),
      processName_(),releaseVersion_(),passID_() { }

    WorkerParams(fhicl::ParameterSet const& procPset,
                 fhicl::ParameterSet const& pset,
                 MasterProductRegistry& reg,
                 ActionTable& actions,
                 std::string const& processName,
                 std::string const & releaseVersion=getReleaseVersion(),
                 std::string const & passID=getPassID()):
      procPset_(&procPset),pset_(&pset),reg_(&reg),actions_(&actions),
      processName_(processName),releaseVersion_(releaseVersion),passID_(passID) { }

    fhicl::ParameterSet const* procPset_;
    fhicl::ParameterSet const* pset_;
    MasterProductRegistry* reg_;
    ActionTable* actions_;
    std::string processName_;
    ReleaseVersion releaseVersion_;
    PassID passID_;
  };

}  // art

#endif /* art_Framework_Core_WorkerParams_h */

// Local Variables:
// mode: c++
// End:
