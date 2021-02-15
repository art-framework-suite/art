#ifndef art_Utilities_SharedResourcesRegistry_h
#define art_Utilities_SharedResourcesRegistry_h
// vim: set sw=2 expandtab :

#include "art/Utilities/SharedResource.h"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace tbb {
  class task_group;
}

namespace hep::concurrency {
  class SerialTaskQueue;
}

namespace art {

  class SharedResourcesRegistry {
  public:
    static SharedResourcesRegistry* instance(bool shutdown = false);
    static detail::SharedResource_t const Legacy;

    SharedResourcesRegistry();
    SharedResourcesRegistry(SharedResourcesRegistry const&) = delete;
    SharedResourcesRegistry(SharedResourcesRegistry&&) = delete;
    SharedResourcesRegistry& operator=(SharedResourcesRegistry const&) = delete;
    SharedResourcesRegistry& operator=(SharedResourcesRegistry&&) = delete;

    bool containsResource(std::string const&) const;
    void registerSharedResource(detail::SharedResource_t const&);
    void registerSharedResource(std::string const&);
    void freeze(tbb::task_group& group);

    using queue_ptr_t = std::shared_ptr<hep::concurrency::SerialTaskQueue>;
    std::vector<queue_ptr_t> createQueues(
      std::vector<std::string> const& resourceNames) const;

  private:
    void ensure_not_frozen(std::string const& name);

    mutable std::recursive_mutex mutex_;
    std::map<std::string, unsigned> resourceCounts_;
    std::vector<std::pair<std::string, queue_ptr_t>> sortedResources_;
    bool frozen_{false};
    unsigned nLegacy_{};
  };
} // namespace art

#endif /* art_Utilities_SharedResourcesRegistry_h */

// Local Variables:
// mode: c++
// End:
