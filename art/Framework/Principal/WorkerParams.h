#ifndef art_Framework_Principal_WorkerParams_h
#define art_Framework_Principal_WorkerParams_h

// This struct is used to communicate parameters into the worker factory.

#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "canvas/Persistency/Provenance/PassID.h"
#include "canvas/Utilities/GetPassID.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

namespace art {
  class ActionTable;

  struct WorkerParams;
}

struct art::WorkerParams {
  WorkerParams(fhicl::ParameterSet const & procPset,
               fhicl::ParameterSet const & pset,
               MasterProductRegistry & reg,
               ActionTable & actions,
               std::string const & processName);

  fhicl::ParameterSet const & procPset_;
  fhicl::ParameterSet const pset_;
  MasterProductRegistry & reg_;
  ActionTable & actions_;
  std::string const processName_;
};

inline
art::WorkerParams::
WorkerParams(fhicl::ParameterSet const & procPset,
             fhicl::ParameterSet const & pset,
             MasterProductRegistry & reg,
             ActionTable & actions,
             std::string const & processName)
  :
  procPset_(procPset),
  pset_(pset),
  reg_(reg),
  actions_(actions),
  processName_(processName)
{
}

#endif /* art_Framework_Principal_WorkerParams_h */

// Local Variables:
// mode: c++
// End:
