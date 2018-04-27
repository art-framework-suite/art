#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Core/detail/SharedModule.h"
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

  class EDFilter : public detail::Filter, public detail::LegacyModule {
  public:
    using ModuleType = EDFilter;
    using WorkerType = WorkerT<EDFilter>;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    bool filterWithScheduleID(Event&, ScheduleID) override final;
    virtual bool filter(Event&) = 0;
  };

  class SharedFilter : public detail::Filter, public detail::SharedModule {
  public:
    using ModuleType = SharedFilter;
    using WorkerType = WorkerT<SharedFilter>;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    bool filterWithScheduleID(Event&, ScheduleID) override final;
    virtual bool filter(Event&, ScheduleID) = 0;
  };

  class ReplicatedFilter : public detail::Filter {
  public:
    using ModuleType = ReplicatedFilter;
    using WorkerType = WorkerT<ReplicatedFilter>;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    bool filterWithScheduleID(Event&, ScheduleID) override final;
    virtual bool filter(Event&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
