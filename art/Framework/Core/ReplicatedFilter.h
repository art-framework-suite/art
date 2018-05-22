#ifndef art_Framework_Core_ReplicatedFilter_h
#define art_Framework_Core_ReplicatedFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

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

#endif /* art_Framework_Core_ReplicatedFilter_h */

// Local Variables:
// mode: c++
// End:
