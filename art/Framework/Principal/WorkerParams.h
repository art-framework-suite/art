#ifndef art_Framework_Principal_WorkerParams_h
#define art_Framework_Principal_WorkerParams_h
// vim: set sw=2 expandtab :

//
// This struct is used to communicate parameters into the worker factory.
//

#include "art/Framework/Core/ModuleType.h"
#include "art/Utilities/ScheduleID.h"
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
                 ActivityRegistry const& actReg,
                 ActionTable const& actions,
                 std::string const& processName,
                 ModuleThreadingType const moduleThreadingType,
                 ScheduleID const si)
      : procPset_{procPset}
      , pset_{pset}
      , reg_{reg}
      , producedProducts_{producedProducts}
      , actReg_{actReg}
      , actions_{actions}
      , processName_{processName}
      , moduleThreadingType_(moduleThreadingType)
      , scheduleID_{si}
    {}

    fhicl::ParameterSet const& procPset_;
    fhicl::ParameterSet const pset_;
    UpdateOutputCallbacks& reg_;
    ProductDescriptions& producedProducts_;
    ActivityRegistry const& actReg_;
    ActionTable const& actions_;
    std::string const processName_;
    ModuleThreadingType moduleThreadingType_;
    ScheduleID scheduleID_;
  };

} // namespace art
#endif /* art_Framework_Principal_WorkerParams_h */

// Local Variables:
// mode: c++
// End:
