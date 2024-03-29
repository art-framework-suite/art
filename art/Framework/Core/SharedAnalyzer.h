#ifndef art_Framework_Core_SharedAnalyzer_h
#define art_Framework_Core_SharedAnalyzer_h
// vim: set sw=2 expandtab :

// ================================================
// The base class for all shared analyzer modules.
// ================================================

#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Analyzer.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Core/fwd.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

  class SharedAnalyzer : public detail::Analyzer, public detail::SharedModule {
  public:
    using ModuleType = SharedAnalyzer;

  protected:
    explicit SharedAnalyzer(fhicl::ParameterSet const& pset);

    template <typename Config>
    explicit SharedAnalyzer(Table<Config> const& config)
      : SharedAnalyzer{config.get_PSet()}
    {}

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
    void beginRunWithFrame(Run const&, ProcessingFrame const&) final;
    void endRunWithFrame(Run const&, ProcessingFrame const&) final;
    void beginSubRunWithFrame(SubRun const&, ProcessingFrame const&) final;
    void endSubRunWithFrame(SubRun const&, ProcessingFrame const&) final;
    void analyzeWithFrame(Event const&, ProcessingFrame const&) final;

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
    virtual void analyze(Event const&, ProcessingFrame const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_SharedAnalyzer_h */

// Local Variables:
// mode: c++
// End:
