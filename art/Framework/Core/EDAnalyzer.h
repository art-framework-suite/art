#ifndef art_Framework_Core_EDAnalyzer_h
#define art_Framework_Core_EDAnalyzer_h
// vim: set sw=2 expandtab :

// ===================================================
// The base class for all legacy analyzer modules.
// ===================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Analyzer.h"
#include "art/Framework/Core/detail/LegacyModule.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
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

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void analyzeWithScheduleID(Event const&, ScheduleID) override final;
    virtual void analyze(Event const&) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_EDAnalyzer_h */

// Local Variables:
// mode: c++
// End:
