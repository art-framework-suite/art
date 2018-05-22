#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

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

} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
