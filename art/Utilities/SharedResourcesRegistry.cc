#include "art/Utilities/SharedResourcesRegistry.h"
// vim: set sw=2 expandtab :

#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
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
    // Propulate queues for known shared resources.  Creating these
    // slots does *not* automatically introduce synchronization.
    // Synchronization is enabled based on the resource-names argument
    // presented to the 'createQueues' member function.
    registerSharedResource(Legacy);
  }

  void
  SharedResourcesRegistry::ensure_not_frozen(std::string const& name)
  {
    std::lock_guard sentry{mutex_};
    if (frozen_) {
      throw art::Exception{art::errors::LogicError, error_context(name)}
        << "The shared-resources registry has been frozen.  All 'serialize' "
           "calls\n"
        << "must be made in the constructor of a shared module and no later.\n";
    }
  }

  void
  SharedResourcesRegistry::registerSharedResource(
    detail::SharedResource_t const& resource)
  {
    registerSharedResource(resource.name);
  }

  void
  SharedResourcesRegistry::registerSharedResource(string const& name)
  {
    ensure_not_frozen(name);
    std::lock_guard sentry{mutex_};
    ++resourceCounts_[name];
    if (name == Legacy.name) {
      ++nLegacy_;
    }
  }

  void
  SharedResourcesRegistry::freeze(tbb::task_group& group)
  {
    frozen_ = true;

    vector<pair<unsigned, string>> resources_sorted_by_count;
    for (auto const& [name, count] : resourceCounts_) {
      // Make sure all non-legacy resources have a higher count, which
      // makes legacy always the first queue.
      auto const offset = (name == Legacy.name) ? 0u : nLegacy_;
      resources_sorted_by_count.emplace_back(count + offset, name);
    }
    cet::sort_all(resources_sorted_by_count, [](auto const& a, auto const& b) {
      return a.first < b.first;
    });

    assert(empty(sortedResources_));
    for (auto const& pr : resources_sorted_by_count) {
      sortedResources_.emplace_back(pr.second, make_shared<SerialTaskQueue>(group));
    }

    // Not needed any more now that we has a sorted list of resources.
    resourceCounts_.clear();
  }

  vector<shared_ptr<SerialTaskQueue>>
  SharedResourcesRegistry::createQueues(
    vector<string> const& resourceNames) const
  {
    std::lock_guard sentry{mutex_};
    std::vector<queue_ptr_t> result;
    if (cet::search_all(resourceNames, Legacy.name)) {
      // This acquirer is for a legacy module, get the queues for all
      // resources.

      // Note: We do not trust legacy modules, they may be accessing
      // one of the shared resources without our knowledge, so we
      // isolate them from the one modules as well as each other.
      for (auto const& pr : sortedResources_) {
        result.push_back(pr.second);
      }
    } else {
      // Not for a legacy module, get the queues for the named
      // resources.
      for (auto const& name : resourceNames) {
        auto it =
          std::find_if(begin(sortedResources_),
                       end(sortedResources_),
                       [&name](auto const& pr) { return pr.first == name; });
        assert(it != sortedResources_.end());
        result.push_back(it->second);
      }
    }

    assert(not empty(result));
    // if (empty(result)) {
    //   // Error, none of the resource names were registered.  Calling
    //   // code is depending on there being at least one shared queue.
    //   return {make_shared<SerialTaskQueue>()};
    // }
    return result;
  }

} // namespace art
