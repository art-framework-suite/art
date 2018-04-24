#ifndef art_Framework_Core_EDProducer_h
#define art_Framework_Core_EDProducer_h
// vim: set sw=2 expandtab :

//
// The base class of "modules" whose main purpose is to
// insert new EDProducts into an Event.
//

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/ProducerImpl.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

  class EDProducer : public detail::ProducerImpl {
    friend class WorkerT<EDProducer>;
  public:
    using ModuleType = EDProducer;
    using WorkerType = WorkerT<EDProducer>;
    static constexpr auto moduleThreadingType() { return ModuleThreadingType::legacy; }
    std::string workerType() const;
  private:
    void doBeginJob();
  };

  class SharedProducer : public detail::ProducerImpl {
    friend class WorkerT<SharedProducer>;
  public:
    using ModuleType = SharedProducer;
    using WorkerType = WorkerT<SharedProducer>;
    static constexpr auto moduleThreadingType() { return ModuleThreadingType::shared; }
    std::string workerType() const;
  private:
    void doBeginJob();
  };

  class ReplicatedProducer : public detail::ProducerImpl {
    friend class WorkerT<ReplicatedProducer>;
  public:
    using ModuleType = ReplicatedProducer;
    using WorkerType = WorkerT<ReplicatedProducer>;
    static constexpr auto moduleThreadingType() { return ModuleThreadingType::replicated; }
    std::string workerType() const;
  private:
    void doBeginJob();
  };

} // namespace art

#endif /* art_Framework_Core_EDProducer_h */

// Local Variables:
// mode: c++
// End:
