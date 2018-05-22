#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/SharedResourcesRegistry.h"

using namespace std;

namespace art {

  string
  EDAnalyzer::workerType() const
  {
    return "WorkerT<EDAnalyzer>";
  }

  void
  EDAnalyzer::setupQueues()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    createQueues();
  }

  void
  EDAnalyzer::analyzeWithScheduleID(Event const& e, ScheduleID const sid)
  {
    ScheduleIDSentry sentry{*this, sid};
    analyze(e);
  }

} // namespace art
