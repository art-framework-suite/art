#include "art/Framework/Core/EDFilter.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/Modifier.h"
#include "art/Framework/Core/SharedResourcesRegistry.h"
#include "art/Framework/Core/WorkerT.h"
#include "art/Framework/Core/detail/get_failureToPut_flag.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/CPCSentry.h"
#include "art/Utilities/ScheduleID.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/RangeSet.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>

using namespace hep::concurrency;
using namespace std;

namespace art {

  string
  EDFilter::workerType() const
  {
    return "WorkerT<EDFilter>";
  }

  void
  EDFilter::setupQueues()
  {
    serialize(SharedResourcesRegistry::kLegacy);
    vector<string> const names(cbegin(resourceNames_), cend(resourceNames_));
    auto queues = SharedResourcesRegistry::instance()->createQueues(names);
    chain_ = new SerialTaskQueueChain{queues};
  }

  bool
  EDFilter::filterWithScheduleID(Event& e, ScheduleID)
  {
    return filter(e);
  }

  string
  SharedFilter::workerType() const
  {
    return "WorkerT<SharedFilter>";
  }

  void
  SharedFilter::setupQueues()
  {
    if (resourceNames_.empty())
      return;

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

  bool
  SharedFilter::filterWithScheduleID(Event& e, ScheduleID const sid)
  {
    return filter(e, sid);
  }

  string
  ReplicatedFilter::workerType() const
  {
    return "WorkerT<ReplicatedFilter>";
  }

  void
  ReplicatedFilter::setupQueues()
  {
    // For art 3.0, replicated modules will not have queues.
  }

  bool
  ReplicatedFilter::filterWithScheduleID(Event& e, ScheduleID)
  {
    return filter(e);
  }

} // namespace art
