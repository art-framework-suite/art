#include "art/Framework/Core/SharedAnalyzer.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  string
  SharedAnalyzer::workerType() const
  {
    return "WorkerT<SharedAnalyzer>";
  }

  void
  SharedAnalyzer::setupQueues()
  {
    createQueues();
  }

  void
  SharedAnalyzer::analyzeWithScheduleID(Event const& e, ScheduleID const sid)
  {
    analyze(e, sid);
  }

} // namespace art
