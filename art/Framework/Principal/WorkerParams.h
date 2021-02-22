#ifndef art_Framework_Principal_WorkerParams_h
#define art_Framework_Principal_WorkerParams_h
// vim: set sw=2 expandtab :

//
// This struct is used to communicate parameters into the worker factory.
//

#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace tbb {
  class task_group;
}

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
                 ScheduleID const sid,
                 tbb::task_group& group)
      : procPset_{procPset}
      , pset_{pset}
      , reg_{reg}
      , producedProducts_{producedProducts}
      , actReg_{actReg}
      , actions_{actions}
      , processName_{processName}
      , scheduleID_{sid}
      , taskGroup_{group}
    {}

    fhicl::ParameterSet const& procPset_;
    fhicl::ParameterSet const pset_;
    UpdateOutputCallbacks& reg_;
    ProductDescriptions& producedProducts_;
    ActivityRegistry const& actReg_;
    ActionTable const& actions_;
    std::string const processName_;
    ScheduleID scheduleID_;
    tbb::task_group& taskGroup_;
  };

} // namespace art
#endif /* art_Framework_Principal_WorkerParams_h */

// Local Variables:
// mode: c++
// End:
