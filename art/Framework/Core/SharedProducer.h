#ifndef art_Framework_Core_SharedProducer_h
#define art_Framework_Core_SharedProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

  class SharedProducer : public detail::Producer, public detail::SharedModule {
  public:
    using ModuleType = SharedProducer;
    using WorkerType = WorkerT<SharedProducer>;

    explicit SharedProducer(fhicl::ParameterSet const& pset)
      : detail::Producer{pset}
    {}

    template <typename Config>
    explicit SharedProducer(Table<Config> const& config)
      : SharedProducer{config.get_PSet()}
    {}

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void produceWithScheduleID(Event&, ScheduleID) override final;
    virtual void produce(Event&, ScheduleID) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_SharedProducer_h */

// Local Variables:
// mode: c++
// End:
