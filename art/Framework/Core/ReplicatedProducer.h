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
    using WorkerType = WorkerT<ReplicatedProducer>;

    // Only the TriggerResults module is allowed to have no
    // module_label parameter.  We provide a default empty string for
    // only that reason.
    explicit ReplicatedProducer(fhicl::ParameterSet const& pset,
                                ProcessingFrame const& frame)
      : detail::Producer{pset}
      , detail::EngineCreator{pset.get<std::string>("module_label", {}),
                              frame.scheduleID()}
    {}

    template <typename Config>
    explicit ReplicatedProducer(Table<Config> const& config,
                                ProcessingFrame const& frame)
      : ReplicatedProducer{config.get_PSet(), frame}
    {}

    using detail::EngineCreator::createEngine;

  private:
    void setupQueues(detail::SharedResources const&) override final;
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
