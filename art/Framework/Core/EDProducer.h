#ifndef art_Framework_Core_EDProducer_h
#define art_Framework_Core_EDProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Principal/WorkerParams.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

  class EDProducer : public detail::Producer {
  public:
    using ModuleType = EDProducer;
    using WorkerType = WorkerT<EDProducer>;

    static constexpr auto
    moduleThreadingType()
    {
      return ModuleThreadingType::legacy;
    }

    std::string workerType() const;

  private:
    void setupQueues() override final;
  };

  class SharedProducer : public detail::Producer {
  public:
    using ModuleType = SharedProducer;
    using WorkerType = WorkerT<SharedProducer>;

    static constexpr auto
    moduleThreadingType()
    {
      return ModuleThreadingType::shared;
    }

    std::string workerType() const;

  private:
    void setupQueues() override final;
  };

  class ReplicatedProducer : public detail::Producer {
  public:
    using ModuleType = ReplicatedProducer;
    using WorkerType = WorkerT<ReplicatedProducer>;

    static constexpr auto
    moduleThreadingType()
    {
      return ModuleThreadingType::replicated;
    }

    std::string workerType() const;

  private:
    void setupQueues() override final;
  };

} // namespace art

#endif /* art_Framework_Core_EDProducer_h */

// Local Variables:
// mode: c++
// End:
