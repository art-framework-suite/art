#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h
// vim: set sw=2 expandtab :

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
    void setupQueues() override final;
    bool filterWithScheduleID(Event&, ScheduleID) override final;
    virtual bool filter(Event&) = 0;
  };

  class SharedFilter : public detail::Filter {
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
    void setupQueues() override final;
    bool filterWithScheduleID(Event&, ScheduleID) override final;
    virtual bool filter(Event&, ScheduleID) = 0;
  };

  class ReplicatedFilter : public detail::Filter {
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
    void setupQueues() override final;
    bool filterWithScheduleID(Event&, ScheduleID) override final;
    virtual bool filter(Event&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
