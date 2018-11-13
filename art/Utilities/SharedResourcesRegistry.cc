#include "art/Utilities/SharedResourcesRegistry.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"
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
using namespace std::string_literals;

namespace art {

  detail::SharedResource_t const SharedResourcesRegistry::Legacy{"__legacy__",
                                                                 false};

  SharedResourcesRegistry::QueueAndCounter::QueueAndCounter()
  {
    counter_ = 0UL;
  }

  SharedResourcesRegistry*
  SharedResourcesRegistry::instance(bool shutdown /*= false*/)
  {
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
    std::string
    error_context(std::string const& name)
    {
      return "An error occurred while attempting to register the shared resource '"s +
             name + "'.\n";
    }
  } // unnamed namespace

  SharedResourcesRegistry::SharedResourcesRegistry()
  {
    frozen_ = false;
    nLegacy_ = 0U;
    // Propulate queues for known shared resources.  Creating these
    // slots does *not* automatically introduce synchronization.
    // Synchronization is enabled based on the resource-names argument
    // presented to the 'createQueues' member function.
    registerSharedResource(Legacy);
  }

  void
  SharedResourcesRegistry::updateSharedResource(string const& name) noexcept(
    false)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    if (frozen_) {
      throw art::Exception{art::errors::LogicError, error_context(name)}
        << "The shared-resources registry has been frozen.  All 'serialize' "
           "calls\n"
        << "must be made in the constructor of a shared module and no later.\n";
    }
    auto it = resourceMap_.find(name);
    if (it == cend(resourceMap_)) {
      throw art::Exception{art::errors::LogicError, error_context(name)}
        << "A 'serialize' call was made for a resource that has not been "
           "registered.\n"
        << "If the resource is an art-based service, make sure that the "
           "service\n"
        << "has been configured for this job.  Otherwise, use the "
           "'serializeExternal'\n"
        << "function call.  If neither of these approaches is appropriate, "
           "contact\n"
        << "artists@fnal.gov.\n";
    }
    auto& queueAndCounter = it->second;
    if (name == Legacy.name) {
      ++nLegacy_;
      // Make sure all non-legacy resources have a higher count, which
      // makes legacy always the first queue.
      for (auto& keyAndVal : resourceMap_) {
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

  void
  SharedResourcesRegistry::registerSharedResource(
    detail::SharedResource_t const& resource)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    resourceMap_[resource.name];
  }

  void
  SharedResourcesRegistry::registerSharedResource(string const& name) noexcept(
    false)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    // Note: This has the intended side-effect of creating the entry
    // if it does not yet exist.
    resourceMap_[name];
    updateSharedResource(name);
  }

  void
  SharedResourcesRegistry::freeze()
  {
    frozen_ = true;
  }

  vector<shared_ptr<SerialTaskQueue>>
  SharedResourcesRegistry::createQueues(string const& resourceName) const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    vector const names{resourceName};
    return createQueues(names);
  }

  vector<shared_ptr<SerialTaskQueue>>
  SharedResourcesRegistry::createQueues(
    vector<string> const& resourceNames) const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    map<pair<unsigned, string>, shared_ptr<SerialTaskQueue>> sortedResources;
    if (cet::search_all(resourceNames, Legacy.name)) {
      // This acquirer is for a legacy module, get the queues for all
      // resources.
      // Note: We do not trust legacy modules, they may be accessing
      // one of the shared resources without our knowledge, so we
      // isolate them from the one modules as well as each other.
      for (auto const& [key, queueAndCounter] : resourceMap_) {
        sortedResources.emplace(make_pair(queueAndCounter.counter_.load(), key),
                                atomic_load(&queueAndCounter.queue_));
      }
    } else {
      // Not for a legacy module, get the queues for the named
      // resources.
      for (auto const& name : resourceNames) {
        auto iter = resourceMap_.find(name);
        assert(iter != resourceMap_.end());
        auto const& [key, queueAndCounter] = *iter;
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
