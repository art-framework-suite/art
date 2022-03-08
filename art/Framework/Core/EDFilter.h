#ifndef art_Framework_Core_EDFilter_h
#define art_Framework_Core_EDFilter_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Filter.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

  class EDFilter : public detail::Filter, private detail::LegacyModule {
  public:
    using ModuleType = EDFilter;

    using detail::LegacyModule::serialTaskQueueChain;
    using detail::LegacyModule::sharedResources;

  protected:
    explicit EDFilter(fhicl::ParameterSet const& pset);

    template <typename Config>
    explicit EDFilter(Table<Config> const& config) : EDFilter{config.get_PSet()}
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
    bool beginRunWithFrame(Run&, ProcessingFrame const&) final;
    bool endRunWithFrame(Run&, ProcessingFrame const&) final;
    bool beginSubRunWithFrame(SubRun&, ProcessingFrame const&) final;
    bool endSubRunWithFrame(SubRun&, ProcessingFrame const&) final;
    bool filterWithFrame(Event&, ProcessingFrame const&) final;

    virtual void beginJob();
    virtual void endJob();
    virtual void respondToOpenInputFile(FileBlock const&);
    virtual void respondToCloseInputFile(FileBlock const&);
    virtual void respondToOpenOutputFiles(FileBlock const&);
    virtual void respondToCloseOutputFiles(FileBlock const&);
    virtual bool beginRun(Run&);
    virtual bool endRun(Run&);
    virtual bool beginSubRun(SubRun&);
    virtual bool endSubRun(SubRun&);
    virtual bool filter(Event&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
