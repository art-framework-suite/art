#ifndef art_Framework_Core_SharedFilter_h
#define art_Framework_Core_SharedFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

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

} // namespace art

#endif /* art_Framework_Core_SharedFilter_h */

// Local Variables:
// mode: c++
// End:
