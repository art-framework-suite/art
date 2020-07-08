#ifndef art_Utilities_SharedResourcesRegistry_h
#define art_Utilities_SharedResourcesRegistry_h
// vim: set sw=2 expandtab :

#include "art/Utilities/SharedResource.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/tsan.h"

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace hep::concurrency {
  class SerialTaskQueue;
}

namespace art {

  class SharedResourcesRegistry {
  private:
    class QueueAndCounter {
    public:
      QueueAndCounter();
      QueueAndCounter(QueueAndCounter const&) = delete;
      QueueAndCounter(QueueAndCounter&&) = delete;
      QueueAndCounter& operator=(QueueAndCounter const&) = delete;
      QueueAndCounter& operator=(QueueAndCounter&&) = delete;

      std::shared_ptr<hep::concurrency::SerialTaskQueue> queue_;
      std::atomic<unsigned long> counter_{0ul};
    };

  public:
    static SharedResourcesRegistry* instance(bool shutdown = false);
    static detail::SharedResource_t const Legacy;

    SharedResourcesRegistry();
    SharedResourcesRegistry(SharedResourcesRegistry const&) = delete;
    SharedResourcesRegistry(SharedResourcesRegistry&&) = delete;
    SharedResourcesRegistry& operator=(SharedResourcesRegistry const&) = delete;
    SharedResourcesRegistry& operator=(SharedResourcesRegistry&&) = delete;

    bool containsResource(std::string const&) const;
    void registerSharedResource(detail::SharedResource_t const&) noexcept(
      false);
    void registerSharedResource(std::string const&) noexcept(false);
    void updateSharedResource(std::string const&) noexcept(false);
    void freeze();
    std::vector<std::shared_ptr<hep::concurrency::SerialTaskQueue>>
    createQueues(std::string const& resourceName) const;
    std::vector<std::shared_ptr<hep::concurrency::SerialTaskQueue>>
    createQueues(std::vector<std::string> const& resourceNames) const;

  private:
    mutable hep::concurrency::RecursiveMutex mutex_{"srr::mutex_"};
    std::map<std::string, QueueAndCounter> resourceMap_;
    bool frozen_;
    unsigned nLegacy_;
  };
} // namespace art

#endif /* art_Utilities_SharedResourcesRegistry_h */

// Local Variables:
// mode: c++
// End:
