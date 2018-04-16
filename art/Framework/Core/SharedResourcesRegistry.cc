#include "art/Framework/Core/SharedResourcesRegistry.h"
// vim: set sw=2 expandtab :

#include "cetlib/container_algorithms.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/SerialTaskQueue.h"
#include "hep_concurrency/tsan.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std;

namespace art {

  string const SharedResourcesRegistry::kLegacy{"__legacy__"};

  SharedResourcesRegistry::QueueAndCounter::~QueueAndCounter()
  {
    queue_.reset();
  }

  SharedResourcesRegistry::QueueAndCounter::QueueAndCounter()
  {
    queue_ = std::make_shared<hep::concurrency::SerialTaskQueue>();
    counter_ = 0UL;
  }

  SharedResourcesRegistry*
  SharedResourcesRegistry::instance(bool shutdown /*= false*/)
  {
    static mutex meMutex_;
    lock_guard<mutex> sentry{meMutex_};
    static SharedResourcesRegistry* me{nullptr};
    if (shutdown) {
      delete me;
      me = nullptr;
      return me;
    }
    if (me == nullptr) {
      me = new SharedResourcesRegistry{};
    }
    return me;
  }

  SharedResourcesRegistry::~SharedResourcesRegistry()
  {
    ANNOTATE_THREAD_IGNORE_BEGIN;
    delete resourceMap_;
    resourceMap_ = nullptr;
    ANNOTATE_THREAD_IGNORE_END;
  }

  class AutoShutdownSharedResourcesRegistry {
  public:
    ~AutoShutdownSharedResourcesRegistry()
    {
      SharedResourcesRegistry::instance(true);
    }
    AutoShutdownSharedResourcesRegistry() = default;
  };

  namespace {
    AutoShutdownSharedResourcesRegistry doAutoShutdown;
  } // unnamed namespace

  SharedResourcesRegistry::SharedResourcesRegistry()
  {
    resourceMap_ = new map<string, QueueAndCounter>;
    nLegacy_ = 0U;
  }

  void
  SharedResourcesRegistry::registerSharedResource(string const& name)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    // Note: This has the intended side-effect of creating the entry
    // if it does not yet exist.
    auto& queueAndCounter = (*resourceMap_)[name];
    if (name == kLegacy) {
      ++nLegacy_;
      // Make sure all non-legacy resources have a higher count, which
      // makes legacy always the first queue.
      for (auto& keyAndVal : *resourceMap_) {
        // Note: keyAndVal.first  is a string (name of resource)
        // Note: keyAndVal.second is a QueueAndCounter
        ++keyAndVal.second.counter_;
      }
      return;
    }
    // count the number of times the resource was registered
    ++queueAndCounter.counter_;
    if (queueAndCounter.counter_.load() == 1) {
      // Make sure all non-legacy resources have a higher count, which
      // makes legacy always the first queue.  When first registering
      // a non-legacy resource, we have to account for any legacy
      // resource registrations already made.
      queueAndCounter.counter_ += nLegacy_;
    }
  }

  vector<shared_ptr<SerialTaskQueue>>
  SharedResourcesRegistry::createQueues(string const& resourceName) const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    vector<string> names{resourceName};
    return createQueues(names);
  }

  vector<shared_ptr<SerialTaskQueue>>
  SharedResourcesRegistry::createQueues(
    vector<string> const& resourceNames) const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    map<pair<unsigned, string>, shared_ptr<SerialTaskQueue>> sortedResources;
    if (cet::search_all(resourceNames, kLegacy)) {
      // This acquirer is for a legacy module, get the queues for all
      // resources.
      // Note: We do not trust legacy modules, they may be accessing
      // one of the shared resources without our knowledge, so we
      // isolate them from the one modules as well as each other.
      for (auto const& keyAndVal : *resourceMap_) {
        auto const& key = keyAndVal.first;
        auto const& queueAndCounter = keyAndVal.second;
        sortedResources.emplace(make_pair(queueAndCounter.counter_.load(), key),
                                atomic_load(&queueAndCounter.queue_));
      }
    } else {
      // Not for a legacy module, get the queues for the named
      // resources.
      for (auto const& name : resourceNames) {
        auto iter = resourceMap_->find(name);
        assert(iter != resourceMap_->end());
        auto const& key = iter->first;
        auto const& queueAndCounter = iter->second;
        sortedResources.emplace(make_pair(queueAndCounter.counter_.load(), key),
                                atomic_load(&queueAndCounter.queue_));
      }
    }
    vector<shared_ptr<SerialTaskQueue>> queues;
    if (sortedResources.empty()) {
      // Error, none of the resource names were registered.  Calling
      // code is depending on there being at least one shared queue.
      queues.emplace_back(make_shared<SerialTaskQueue>());
    } else {
      // At least some of the named resources exist.
      queues.reserve(sortedResources.size());
      for (auto const& keyAndVal : sortedResources) {
        // Note: keyAndVal.second is a shared_ptr<SerialTaskQueue>
        queues.push_back(keyAndVal.second);
      }
    }
    return queues;
  }

} // namespace art
