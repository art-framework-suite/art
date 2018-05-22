#ifndef art_Framework_Core_SharedAnalyzer_h
#define art_Framework_Core_SharedAnalyzer_h
// vim: set sw=2 expandtab :

// ================================================
// The base class for all shared analyzer modules.
// ================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/Analyzer.h"
#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"

#include <string>

namespace art {

  class SharedAnalyzer : public detail::Analyzer, public detail::SharedModule {
  public:
    using WorkerType = WorkerT<SharedAnalyzer>;
    using ModuleType = SharedAnalyzer;

    using detail::Analyzer::Analyzer;

    std::string workerType() const;

  private:
    void setupQueues() override final;
    void analyzeWithScheduleID(Event const&, ScheduleID) override final;
    virtual void analyze(Event const&, ScheduleID) = 0;
  };

} // namespace art

#endif /* art_Framework_Core_SharedAnalyzer_h */

// Local Variables:
// mode: c++
// End:
