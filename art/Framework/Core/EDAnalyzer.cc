#include "art/Framework/Core/EDAnalyzer.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Observer.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>

using namespace hep::concurrency;
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
