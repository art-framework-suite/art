#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "fhiclcpp/ParameterSet.h"

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>

namespace art {

  class EDFilter : public detail::Filter, private detail::LegacyModule {
  public:
    using ModuleType = EDFilter;
    using WorkerType = WorkerT<EDFilter>;

    EDFilter() = default;
    explicit EDFilter(fhicl::ParameterSet const& pset)
      : detail::Filter{pset}
      , detail::LegacyModule{pset.get<std::string>("module_label")}
    {}

    template <typename Config>
    explicit EDFilter(Table<Config> const& config) : EDFilter{config.get_PSet()}
    {}

    using detail::LegacyModule::createEngine;
    using detail::LegacyModule::scheduleID;
    using detail::LegacyModule::serialTaskQueueChain;

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

    explicit SharedFilter(fhicl::ParameterSet const& pset)
      : detail::Filter{pset}
    {}

    template <typename Config>
    explicit SharedFilter(Table<Config> const& config)
      : SharedFilter{config.get_PSet()}
    {}

    std::string workerType() const;

  private:
    void setupQueues() override final;
    bool filterWithScheduleID(Event&, ScheduleID) override final;
    virtual bool filter(Event&, ScheduleID) = 0;
  };

  class ReplicatedFilter : public detail::Filter,
                           private detail::EngineCreator {
  public:
    using ModuleType = ReplicatedFilter;
    using WorkerType = WorkerT<ReplicatedFilter>;

    std::string workerType() const;

    explicit ReplicatedFilter(fhicl::ParameterSet const& pset,
                              ScheduleID const scheduleID)
      : detail::EngineCreator{pset.get<std::string>("module_label"), scheduleID}
    {}

    template <typename Config>
    explicit ReplicatedFilter(Table<Config> const& config,
                              ScheduleID const scheduleID)
      : ReplicatedFilter{config.get_PSet(), scheduleID}
    {}

    using detail::EngineCreator::createEngine;

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
