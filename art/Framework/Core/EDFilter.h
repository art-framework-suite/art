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
    using WorkerType = WorkerT<EDFilter>;

    explicit EDFilter(fhicl::ParameterSet const& pset)
      : detail::Filter{pset}
      , detail::LegacyModule{pset.get<std::string>("module_label")}
    {}

    template <typename Config>
    explicit EDFilter(Table<Config> const& config) : EDFilter{config.get_PSet()}
    {}

    using detail::LegacyModule::createEngine;
    using detail::LegacyModule::scheduleID;
    using detail::LegacyModule::serialTaskQueueChain;
    using detail::LegacyModule::sharedResources;

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
    bool beginRunWithFrame(Run&, ProcessingFrame const&) override final;
    bool endRunWithFrame(Run&, ProcessingFrame const&) override final;
    bool beginSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    bool endSubRunWithFrame(SubRun&, ProcessingFrame const&) override final;
    bool filterWithFrame(Event&, ProcessingFrame const&) override final;

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

  extern template class WorkerT<EDFilter>;

} // namespace art

#endif /* art_Framework_Core_EDFilter_h */

// Local Variables:
// mode: c++
// End:
