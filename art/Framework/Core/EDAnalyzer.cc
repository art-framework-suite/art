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
  EDAnalyzer::doBeginJob()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    vector<string> const names(cbegin(resourceNames_), cend(resourceNames_));
    auto queues = SharedResourcesRegistry::instance()->createQueues(names);
    chain_ = new SerialTaskQueueChain{queues};
    beginJob();
  }

  string
  SharedAnalyzer::workerType() const
  {
    return "WorkerT<SharedAnalyzer>";
  }

  void
  SharedAnalyzer::doBeginJob()
  {
    if (!resourceNames_.empty()) {
      if (asyncDeclared_) {
        throw art::Exception{
          art::errors::LogicError,
          "An error occurred while processing scheduling options for a module."}
          << "async<InEvent>() cannot be called in combination with any "
             "serialize<InEvent>(...) calls.\n";
      }
      vector<string> const names(cbegin(resourceNames_), cend(resourceNames_));
      auto queues = SharedResourcesRegistry::instance()->createQueues(names);
      chain_ = new SerialTaskQueueChain{queues};
    }
    beginJob();
  }

  string
  ReplicatedAnalyzer::workerType() const
  {
    return "WorkerT<ReplicatedAnalyzer>";
  }

  void
  ReplicatedAnalyzer::doBeginJob()
  {
    beginJob();
  }

} // namespace art
