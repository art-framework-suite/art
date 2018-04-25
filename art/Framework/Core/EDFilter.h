#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h
// vim: set sw=2 expandtab :

//
// The base class of all "modules" used to control the flow of
// processing in a trigger path.  Filters can also insert products
// into the event.  These products should be informational products
// about the filter decision.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

namespace art {

  class EDFilter : public detail::Filter {
    friend class WorkerT<EDFilter>;

  public:
    using ModuleType = EDFilter;
    using WorkerType = WorkerT<EDFilter>;
    static constexpr ModuleThreadingType
    moduleThreadingType()
    {
      return ModuleThreadingType::legacy;
    }
    std::string workerType() const;

  private:
    void doBeginJob();
  };

  class SharedFilter : public detail::Filter {
    friend class WorkerT<SharedFilter>;

  public:
    using ModuleType = SharedFilter;
    using WorkerType = WorkerT<SharedFilter>;

    static constexpr ModuleThreadingType
    moduleThreadingType()
    {
      return ModuleThreadingType::shared;
    }

    std::string workerType() const;

  private:
    void doBeginJob();
  };

  class ReplicatedFilter : public detail::Filter {
    friend class WorkerT<ReplicatedFilter>;

  public:
    using ModuleType = ReplicatedFilter;
    using WorkerType = WorkerT<ReplicatedFilter>;

    static constexpr ModuleThreadingType
    moduleThreadingType()
    {
      return ModuleThreadingType::replicated;
    }

    std::string workerType() const;

  private:
    void doBeginJob();
  };

} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
