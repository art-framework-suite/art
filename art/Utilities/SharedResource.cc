#include "art/Utilities/SharedResource.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "cetlib_except/demangle.h"
#include "hep_concurrency/SerialTaskQueue.h"
#include "range/v3/view.hpp"

#include <algorithm>
#include <cassert>
#include <memory>
#include <string>
#include <utility>
#include <vector>

using namespace hep::concurrency;
using namespace std::string_literals;

namespace {
  std::string
  error_context(std::string const& name)
  {
    return "An error occurred while attempting to register the shared resource '"s +
           name + "'.\n";
  }
}

namespace art::detail {
  SharedResource_t::SharedResource_t(std::string const& resource_name,
                                     bool const demangle)
    : name{demangle ? cet::demangle_symbol(resource_name) : resource_name}
  {}

  SharedResource_t::~SharedResource_t() = default;

  SharedResource_t const LegacyResource{"__legacy__", false};

  SharedResources::SharedResources()
  {
    // Propulate queues for known shared resources.  Creating these
    // slots does *not* automatically introduce synchronization.
    // Synchronization is enabled based on the resource-names argument
    // presented to the 'createQueues' member function.
    registerSharedResource(LegacyResource);
  }

  void
  SharedResources::ensure_not_frozen(std::string const& name)
  {
    if (frozen_) {
      throw Exception{errors::LogicError, error_context(name)}
        << "The shared-resources registry has been frozen.  All 'serialize' "
           "calls\n"
        << "must be made in the constructor of a shared module and no later.\n";
    }
  }

  void
  SharedResources::registerSharedResources(std::set<std::string> const& names)
  {
    cet::for_all(names, [this](auto const& name) { register_resource(name); });
  }

  void
  SharedResources::registerSharedResource(SharedResource_t const& resource)
  {
    register_resource(resource.name);
  }

  void
  SharedResources::register_resource(std::string const& name)
  {
    ensure_not_frozen(name);
    ++resourceCounts_[name];
    if (name == LegacyResource.name) {
      ++nLegacy_;
    }
  }

  void
  SharedResources::freeze(tbb::task_group& group)
  {
    frozen_ = true;

    std::vector<std::pair<unsigned, std::string>> resources_sorted_by_count;
    for (auto const& [name, count] : resourceCounts_) {
      // Make sure all non-legacy resources have a higher count, which
      // makes legacy always the first queue.
      auto const offset = (name == LegacyResource.name) ? 0u : nLegacy_;
      resources_sorted_by_count.emplace_back(count + offset, name);
    }
    cet::sort_all(resources_sorted_by_count, [](auto const& a, auto const& b) {
      return a.first < b.first;
    });

    using namespace ranges;
    sortedResources_ =
      resources_sorted_by_count | views::values |
      views::transform([&group](auto const& key) {
        return std::pair{key, std::make_shared<SerialTaskQueue>(group)};
      }) |
      to<std::vector>();

    // Not needed any more now that we have a sorted list of resources.
    resourceCounts_.clear();
  }

  std::vector<std::shared_ptr<SerialTaskQueue>>
  SharedResources::createQueues(
    std::vector<std::string> const& resourceNames) const
  {
    using namespace ranges;
    if (cet::search_all(resourceNames, LegacyResource.name)) {
      // We do not trust legacy modules as they may be accessing one
      // of the shared resources without our knowledge.  We therefore
      // isolate them from all other shared modules (and each other).
      return sortedResources_ | views::values | to<std::vector>();
    }
    std::vector<std::shared_ptr<SerialTaskQueue>> result;
    // Not for a legacy module, get the queues for the named resources.
    for (auto const& name : resourceNames) {
      auto it =
        std::find_if(begin(sortedResources_),
                     end(sortedResources_),
                     [&name](auto const& pr) { return pr.first == name; });
      assert(it != sortedResources_.end());
      result.push_back(it->second);
    }

    assert(not empty(result));
    return result;
  }

} // namespace art
