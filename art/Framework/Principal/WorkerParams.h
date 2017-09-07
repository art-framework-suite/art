#ifndef art_Framework_Principal_WorkerParams_h
#define art_Framework_Principal_WorkerParams_h
// vim: set sw=2 expandtab :

//
// This struct is used to communicate parameters into the worker factory.
//

#include "art/Framework/Core/ModuleType.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

class ActionTable;
class ActivityRegistry;
class MasterProductRegistry;

struct WorkerParams {

  WorkerParams(fhicl::ParameterSet& procPset, fhicl::ParameterSet const& pset, MasterProductRegistry& reg,
               ActionTable& actions, ActivityRegistry& actReg, std::string const& processName,
               ModuleThreadingType moduleThreadingType, int streamIndex)
    : procPset_(procPset)
    , pset_(pset)
    , reg_(reg)
    , actions_(actions)
    , actReg_(actReg)
    , processName_(processName)
    , moduleThreadingType_(moduleThreadingType)
    , streamIndex_(streamIndex)
  {
  }

  fhicl::ParameterSet const&
  procPset_;

  fhicl::ParameterSet
  pset_;

  MasterProductRegistry&
  reg_;

  ActionTable&
  actions_;

  ActivityRegistry&
  actReg_;

  std::string const
  processName_;

  ModuleThreadingType
  moduleThreadingType_;

  int
  streamIndex_;

};


} // namespace art

#endif /* art_Framework_Principal_WorkerParams_h */

// Local Variables:
// mode: c++
// End:
