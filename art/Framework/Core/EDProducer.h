#ifndef art_Framework_Core_EDProducer_h
#define art_Framework_Core_EDProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {

  class EDProducer : public detail::Producer, private detail::LegacyModule {
  public:
    using ModuleType = EDProducer;

    using detail::LegacyModule::serialTaskQueueChain;
    using detail::LegacyModule::sharedResources;

  protected:
    explicit EDProducer(fhicl::ParameterSet const& pset);

    template <typename Config>
    explicit EDProducer(Table<Config> const& config)
      : EDProducer{config.get_PSet()}
    {}

    using detail::LegacyModule::createEngine;
    using detail::LegacyModule::scheduleID;

  private:
    std::unique_ptr<Worker> doMakeWorker(WorkerParams const& wp) final;
    void setupQueues(detail::SharedResources const& resources) final;
    void beginJobWithFrame(ProcessingFrame const&) final;
    void endJobWithFrame(ProcessingFrame const&) final;
    void respondToOpenInputFileWithFrame(FileBlock const&,
                                         ProcessingFrame const&) final;
    void respondToCloseInputFileWithFrame(FileBlock const&,
                                          ProcessingFrame const&) final;
    void respondToOpenOutputFilesWithFrame(FileBlock const&,
                                           ProcessingFrame const&) final;
    void respondToCloseOutputFilesWithFrame(FileBlock const&,
                                            ProcessingFrame const&) final;
    void beginRunWithFrame(Run&, ProcessingFrame const&) final;
    void endRunWithFrame(Run&, ProcessingFrame const&) final;
    void beginSubRunWithFrame(SubRun&, ProcessingFrame const&) final;
    void endSubRunWithFrame(SubRun&, ProcessingFrame const&) final;
    void produceWithFrame(Event&, ProcessingFrame const&) final;

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
