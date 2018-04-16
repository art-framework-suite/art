#ifndef art_Framework_Core_SharedResourcesRegistry_h
#define art_Framework_Core_SharedResourcesRegistry_h
// vim: set sw=2 expandtab :

#include "hep_concurrency/RecursiveMutex.h"
#include "hep_concurrency/SerialTaskQueue.h"
#include "hep_concurrency/tsan.h"

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace art {
  // <Singleton>
  class SharedResourcesRegistry {
  private: // TYPES
    class QueueAndCounter {
    public: // Special Member Functions
      ~QueueAndCounter();
      QueueAndCounter();
      QueueAndCounter(QueueAndCounter const&) = delete;
      QueueAndCounter(QueueAndCounter&&) = delete;
      QueueAndCounter& operator=(QueueAndCounter const&) = delete;
      QueueAndCounter& operator=(QueueAndCounter&&) = delete;

    public: // Member Data
      std::shared_ptr<hep::concurrency::SerialTaskQueue> queue_{
        std::make_shared<hep::concurrency::SerialTaskQueue>()};
      std::atomic<unsigned long> counter_;
    };

  public: // STATIC MEMBER FUNCTIONS
    static SharedResourcesRegistry* instance(bool shutdown = false);

  public: // STATIC MEMBER DATA
    static std::string const kLegacy;

  private: // MEMBER FUNCTIONS -- Special Member Functions
    ~SharedResourcesRegistry();
    SharedResourcesRegistry();
    SharedResourcesRegistry(SharedResourcesRegistry const&) = delete;
    SharedResourcesRegistry(SharedResourcesRegistry&&) = delete;
    SharedResourcesRegistry& operator=(SharedResourcesRegistry const&) = delete;
    SharedResourcesRegistry& operator=(SharedResourcesRegistry&&) = delete;

  public: // MEMBER FUNCTIONS
    void registerSharedResource(std::string const&);
    std::vector<std::shared_ptr<hep::concurrency::SerialTaskQueue>>
    createQueues(std::string const& resourceName) const;
    std::vector<std::shared_ptr<hep::concurrency::SerialTaskQueue>>
    createQueues(std::vector<std::string> const& resourceNames) const;

  private: // MEMBER DATA
    mutable hep::concurrency::RecursiveMutex mutex_{"srr::mutex_"};
    std::map<std::string, QueueAndCounter>* resourceMap_;
    unsigned nLegacy_;
  };
} // namespace art

#endif /* art_Framework_Core_SharedResourcesRegistry_h */

// Local Variables:
// mode: c++
// End:
