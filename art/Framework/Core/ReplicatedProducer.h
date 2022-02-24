#ifndef art_Framework_Core_ReplicatedProducer_h
#define art_Framework_Core_ReplicatedProducer_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ProcessingFrame.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Core/detail/Producer.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"

#include <string>

namespace art {
  namespace detail {
    class SharedResources;
  }

  class ReplicatedProducer : public detail::Producer,
                             private detail::EngineCreator {
  public:
    using ModuleType = ReplicatedProducer;

  protected:
    explicit ReplicatedProducer(fhicl::ParameterSet const& pset,
                                ProcessingFrame const& frame);

    template <typename Config>
    explicit ReplicatedProducer(Table<Config> const& config,
                                ProcessingFrame const& frame)
      : ReplicatedProducer{config.get_PSet(), frame}
    {}

    using detail::EngineCreator::createEngine;

  private:
    std::unique_ptr<Worker> doMakeWorker(WorkerParams const& wp) final;
    void setupQueues(detail::SharedResources const&) final;
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

    virtual void beginJob(ProcessingFrame const&);
    virtual void endJob(ProcessingFrame const&);
    virtual void respondToOpenInputFile(FileBlock const&,
                                        ProcessingFrame const&);
    virtual void respondToCloseInputFile(FileBlock const&,
                                         ProcessingFrame const&);
    virtual void respondToOpenOutputFiles(FileBlock const&,
                                          ProcessingFrame const&);
    virtual void respondToCloseOutputFiles(FileBlock const&,
                                           ProcessingFrame const&);
    virtual void beginRun(Run const&, ProcessingFrame const&);
    virtual void endRun(Run const&, ProcessingFrame const&);
    virtual void beginSubRun(SubRun const&, ProcessingFrame const&);
    virtual void endSubRun(SubRun const&, ProcessingFrame const&);
    virtual void produce(Event&, ProcessingFrame const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_ReplicatedProducer_h */

// Local Variables:
// mode: c++
// End:
