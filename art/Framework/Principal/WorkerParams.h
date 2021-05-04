#ifndef art_Framework_Principal_WorkerParams_h
#define art_Framework_Principal_WorkerParams_h
// vim: set sw=2 expandtab :

//
// This struct is used to communicate parameters into the worker factory.
//

#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "fhiclcpp/ParameterSet.h"

#include <tbb/task_group.h> // Can't forward-declare this class.

namespace art {

  class ActionTable;
  class ActivityRegistry;
  class UpdateOutputCallbacks;
  namespace detail {
    class SharedResources;
  }

  struct WorkerParams {
    UpdateOutputCallbacks& reg_;
    ProductDescriptions& producedProducts_;
    ActivityRegistry const& actReg_;
    ActionTable const& actions_;
    ScheduleID scheduleID_;
    tbb::task_group& taskGroup_;
    detail::SharedResources& resources_;
  };

} // namespace art

#endif /* art_Framework_Principal_WorkerParams_h */

// Local Variables:
// mode: c++
// End:
