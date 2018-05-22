#ifndef art_Framework_Core_ReplicatedProducer_h
#define art_Framework_Core_ReplicatedProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

  class ReplicatedProducer : public detail::Producer,
                             private detail::EngineCreator {
  public:
    using ModuleType = ReplicatedProducer;
    using WorkerType = WorkerT<ReplicatedProducer>;

    // Only the TriggerResults module is allowed to have no
    // module_label parameter.  We provide a default empty string for
    // only that reason.
    explicit ReplicatedProducer(fhicl::ParameterSet const& pset,
                                ScheduleID const scheduleID)
      : detail::EngineCreator{pset.get<std::string>("module_label", {}),
                              scheduleID}
    {}

    template <typename Config>
    explicit ReplicatedProducer(Table<Config> const& config,
                                ScheduleID const scheduleID)
      : ReplicatedProducer{config.get_PSet(), scheduleID}
    {}

    using detail::EngineCreator::createEngine;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void produceWithScheduleID(Event&, ScheduleID) override final;
    virtual void produce(Event&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_ReplicatedProducer_h */

// Local Variables:
// mode: c++
// End:
