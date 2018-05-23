#ifndef art_Framework_Core_EDProducer_h
#define art_Framework_Core_EDProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

  class EDProducer : public detail::Producer, private detail::LegacyModule {
  public:
    using ModuleType = EDProducer;
    using WorkerType = WorkerT<EDProducer>;

    EDProducer() = default;
    explicit EDProducer(fhicl::ParameterSet const& pset)
      : detail::Producer{pset}
      , detail::LegacyModule{pset.get<std::string>("module_label")}
    {}

    template <typename Config>
    explicit EDProducer(Table<Config> const& config)
      : EDProducer{config.get_PSet()}
    {}

    using detail::LegacyModule::createEngine;
    using detail::LegacyModule::scheduleID;
    using detail::LegacyModule::serialTaskQueueChain;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void beginJobWithServices(Services const&) override final;
    void endJobWithServices(Services const&) override final;
    void respondToOpenInputFileWithServices(FileBlock const&,
                                            Services const&) override final;
    void respondToCloseInputFileWithServices(FileBlock const&,
                                             Services const&) override final;
    void respondToOpenOutputFilesWithServices(FileBlock const&,
                                              Services const&) override final;
    void respondToCloseOutputFilesWithServices(FileBlock const&,
                                               Services const&) override final;
    void beginRunWithServices(Run&, Services const&) override final;
    void endRunWithServices(Run&, Services const&) override final;
    void beginSubRunWithServices(SubRun&, Services const&) override final;
    void endSubRunWithServices(SubRun&, Services const&) override final;
    void produceWithServices(Event&, Services const&) override final;

    virtual void beginJob();
    virtual void endJob();
    virtual void respondToOpenInputFile(FileBlock const&);
    virtual void respondToCloseInputFile(FileBlock const&);
    virtual void respondToOpenOutputFiles(FileBlock const&);
    virtual void respondToCloseOutputFiles(FileBlock const&);
    virtual void beginRun(Run&);
    virtual void endRun(Run&);
    virtual void beginSubRun(SubRun&);
    virtual void endSubRun(SubRun&);
    virtual void produce(Event&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_EDProducer_h */

// Local Variables:
// mode: c++
// End:
