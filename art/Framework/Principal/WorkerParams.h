#ifndef art_Framework_Principal_WorkerParams_h
#define art_Framework_Principal_WorkerParams_h
// vim: set sw=2 expandtab :

//
// This struct is used to communicate parameters into the worker factory.
//

#include "art/Framework/Core/ModuleType.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

class ActionTable;
class ActivityRegistry;
class UpdateOutputCallbacks;

struct WorkerParams {

  WorkerParams(fhicl::ParameterSet const& procPset,
               fhicl::ParameterSet const& pset,
               UpdateOutputCallbacks& reg,
               ProductDescriptions& producedProducts,
               ActivityRegistry& actReg,
               ActionTable& actions,
               std::string const& processName,
               ModuleThreadingType const moduleThreadingType,
               int const streamIndex)
    : procPset_{procPset}
    , pset_{pset}
    , reg_{reg}
    , producedProducts_{producedProducts}
    , actReg_{actReg}
    , actions_{actions}
    , processName_{processName}
    , moduleThreadingType_(moduleThreadingType)
    , streamIndex_(streamIndex)
  {}

  fhicl::ParameterSet const& procPset_;
  fhicl::ParameterSet const pset_;
  UpdateOutputCallbacks& reg_;
  ProductDescriptions& producedProducts_;
  ActivityRegistry& actReg_;
  ActionTable& actions_;
  std::string const processName_;
  ModuleThreadingType moduleThreadingType_;
  int streamIndex_;

};

} // namespace art
#endif /* art_Framework_Principal_WorkerParams_h */

// Local Variables:
// mode: c++
// End:
