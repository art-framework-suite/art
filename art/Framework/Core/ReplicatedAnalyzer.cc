#include "art/Framework/Core/ReplicatedAnalyzer.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  ReplicatedAnalyzer::workerType() const
  {
    return "WorkerT<ReplicatedAnalyzer>";
  }

  void
  ReplicatedAnalyzer::setupQueues()
  {
    // For art 3.0, replicated modules will not have queues.
  }

  void
  ReplicatedAnalyzer::analyzeWithScheduleID(Event const& e, ScheduleID)
  {
    analyze(e);
  }

} // namespace art
