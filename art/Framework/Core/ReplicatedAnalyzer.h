#ifndef art_Framework_Core_ReplicatedAnalyzer_h
#define art_Framework_Core_ReplicatedAnalyzer_h
// vim: set sw=2 expandtab :

// =====================================================
// The base class for all replicated analyzer modules.
// =====================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Analyzer.h"
#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

  class ReplicatedAnalyzer : public detail::Analyzer,
                             private detail::EngineCreator {
  public:
    using WorkerType = WorkerT<ReplicatedAnalyzer>;
    using ModuleType = ReplicatedAnalyzer;

    explicit ReplicatedAnalyzer(fhicl::ParameterSet const& pset,
                                ScheduleID const scheduleID)
      : detail::Analyzer{pset}
      , detail::EngineCreator{pset.get<std::string>("module_label"), scheduleID}
    {}

    template <typename Config>
    explicit ReplicatedAnalyzer(Table<Config> const& config,
                                ScheduleID const scheduleID)
      : ReplicatedAnalyzer{config.get_PSet(), scheduleID}
    {}

    using detail::EngineCreator::createEngine;

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
    void beginRunWithServices(Run const&, Services const&) override final;
    void endRunWithServices(Run const&, Services const&) override final;
    void beginSubRunWithServices(SubRun const&, Services const&) override final;
    void endSubRunWithServices(SubRun const&, Services const&) override final;
    void analyzeWithServices(Event const&, Services const&) override final;

    virtual void beginJob(Services const&);
    virtual void endJob(Services const&);
    virtual void respondToOpenInputFile(FileBlock const&, Services const&);
    virtual void respondToCloseInputFile(FileBlock const&, Services const&);
    virtual void respondToOpenOutputFiles(FileBlock const&, Services const&);
    virtual void respondToCloseOutputFiles(FileBlock const&, Services const&);
    virtual void beginRun(Run const&, Services const&);
    virtual void endRun(Run const&, Services const&);
    virtual void beginSubRun(SubRun const&, Services const&);
    virtual void endSubRun(SubRun const&, Services const&);
    virtual void analyze(Event const&, Services const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_ReplicatedAnalyzer_h */

// Local Variables:
// mode: c++
// End:
