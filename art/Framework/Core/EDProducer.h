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
    using detail::LegacyModule::sharedResources;

    std::string workerType() const;

  private:
    void setupQueues(detail::SharedResources const& resources) override final;
    void beginJobWithFrame(ProcessingFrame const&) override final;
    void endJobWithFrame(ProcessingFrame const&) override final;
    void respondToOpenInputFileWithFrame(FileBlock const&,
                                         ProcessingFrame const&) override final;
    void respondToCloseInputFileWithFrame(
      FileBlock const&,
      ProcessingFrame const&) override final;
    void respondToOpenOutputFilesWithFrame(
      FileBlock const&,
      ProcessingFrame const&) override final;
    void respondToCloseOutputFilesWithFrame(
      FileBlock const&,
      ProcessingFrame const&) override final;
    void beginRunWithFrame(Run&, ProcessingFrame const&) override final;
    void endRunWithFrame(Run&, ProcessingFrame const&) override final;
    void beginSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    void endSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    void produceWithFrame(Event&, ProcessingFrame const&) override final;

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
