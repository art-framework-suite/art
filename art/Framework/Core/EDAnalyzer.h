#ifndef art_Framework_Core_EDAnalyzer_h
#define art_Framework_Core_EDAnalyzer_h
// vim: set sw=2 expandtab :

// ===================================================
// The base class for all legacy analyzer modules.
// ===================================================

#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Analyzer.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

  class EDAnalyzer : public detail::Analyzer, private detail::LegacyModule {
  public:
    using WorkerType = WorkerT<EDAnalyzer>;
    using ModuleType = EDAnalyzer;

    explicit EDAnalyzer(fhicl::ParameterSet const& pset)
      : detail::Analyzer{pset}
      , detail::LegacyModule{pset.get<std::string>("module_label")}
    {}

    template <typename Config>
    explicit EDAnalyzer(Table<Config> const& config)
      : EDAnalyzer{config.get_PSet()}
    {}

    using detail::LegacyModule::createEngine;
    using detail::LegacyModule::scheduleID;
    using detail::LegacyModule::serialTaskQueueChain;
    using detail::LegacyModule::sharedResources;

  private:
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
    void beginRunWithFrame(Run const&, ProcessingFrame const&) final;
    void endRunWithFrame(Run const&, ProcessingFrame const&) final;
    void beginSubRunWithFrame(SubRun const&, ProcessingFrame const&) final;
    void endSubRunWithFrame(SubRun const&, ProcessingFrame const&) final;
    void analyzeWithFrame(Event const&, ProcessingFrame const&) final;

    virtual void beginJob();
    virtual void endJob();
    virtual void respondToOpenInputFile(FileBlock const&);
    virtual void respondToCloseInputFile(FileBlock const&);
    virtual void respondToOpenOutputFiles(FileBlock const&);
    virtual void respondToCloseOutputFiles(FileBlock const&);
    virtual void beginRun(Run const&);
    virtual void endRun(Run const&);
    virtual void beginSubRun(SubRun const&);
    virtual void endSubRun(SubRun const&);
    virtual void analyze(Event const&) = 0;
  };

  extern template class WorkerT<EDAnalyzer>;

} // namespace art

#endif /* art_Framework_Core_EDAnalyzer_h */

// Local Variables:
// mode: c++
// End:
