#ifndef art_Framework_Core_EDProducer_h
#define art_Framework_Core_EDProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

  class EDProducer : public detail::Producer, public detail::LegacyModule {
  public:
    using ModuleType = EDProducer;
    using WorkerType = WorkerT<EDProducer>;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void produceWithScheduleID(Event&, ScheduleID) override final;
    virtual void produce(Event&) = 0;
  };

  class SharedProducer : public detail::Producer, public detail::SharedModule {
  public:
    using ModuleType = SharedProducer;
    using WorkerType = WorkerT<SharedProducer>;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void produceWithScheduleID(Event&, ScheduleID) override final;
    virtual void produce(Event&, ScheduleID) = 0;
  };

  class ReplicatedProducer : public detail::Producer {
  public:
    using ModuleType = ReplicatedProducer;
    using WorkerType = WorkerT<ReplicatedProducer>;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void produceWithScheduleID(Event&, ScheduleID) override final;
    virtual void produce(Event&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_EDProducer_h */

// Local Variables:
// mode: c++
// End:
