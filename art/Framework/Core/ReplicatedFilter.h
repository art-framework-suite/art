#ifndef art_Framework_Core_ReplicatedFilter_h
#define art_Framework_Core_ReplicatedFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ProcessingFrame.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

  class ReplicatedFilter : public detail::Filter,
                           private detail::EngineCreator {
  public:
    using ModuleType = ReplicatedFilter;
    using WorkerType = WorkerT<ReplicatedFilter>;

    explicit ReplicatedFilter(fhicl::ParameterSet const& pset,
                              ProcessingFrame const& frame)
      : detail::Filter{pset}
      , detail::EngineCreator{pset.get<std::string>("module_label"),
                              frame.scheduleID()}
    {}

    template <typename Config>
    explicit ReplicatedFilter(Table<Config> const& config,
                              ProcessingFrame const& frame)
      : ReplicatedFilter{config.get_PSet(), frame}
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
    bool beginRunWithFrame(Run&, ProcessingFrame const&) override final;
    bool endRunWithFrame(Run&, ProcessingFrame const&) override final;
    bool beginSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    bool endSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    bool filterWithFrame(Event&, ProcessingFrame const&) override final;

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
    virtual bool filter(Event&, ProcessingFrame const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_ReplicatedFilter_h */

// Local Variables:
// mode: c++
// End:
