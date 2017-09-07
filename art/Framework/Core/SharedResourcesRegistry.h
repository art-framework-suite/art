#ifndef art_Framework_Core_SharedResourcesRegistry_h
#define art_Framework_Core_SharedResourcesRegistry_h
// vim: set sw=2 expandtab :

#include "hep_concurrency/SerialTaskQueue.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace art {

// <Singleton>
class SharedResourcesRegistry {

private: // TYPES

  class QueueAndCounter {

  public:

    ~QueueAndCounter();

    QueueAndCounter();

    QueueAndCounter(QueueAndCounter const&) = delete;

    QueueAndCounter(QueueAndCounter&&) = delete;

    QueueAndCounter&
    operator=(QueueAndCounter const&) = delete;

    QueueAndCounter&
    operator=(QueueAndCounter&&) = delete;

  public:

    std::shared_ptr<hep::concurrency::SerialTaskQueue>
    queue_{};

    unsigned long
    counter_{0UL};

  };

public: // STATIC MEMBER FUNCTIONS

  static
  SharedResourcesRegistry*
  instance();
  
public: // STATIC MEMBER DATA

  static
  std::string const
  kLegacy;
  
private: // MEMBER FUNCTIONS -- Special Member Functions

  ~SharedResourcesRegistry();

  SharedResourcesRegistry();

  SharedResourcesRegistry(SharedResourcesRegistry const&) = delete;
  
  SharedResourcesRegistry&
  operator=(SharedResourcesRegistry const&) = delete;
  
  SharedResourcesRegistry(SharedResourcesRegistry&&) = delete;

  SharedResourcesRegistry&
  operator=(SharedResourcesRegistry&&) = delete;
  
public: // MEMBER FUNCTIONS
  
  void
  registerSharedResource(std::string const&);

  std::vector<std::shared_ptr<hep::concurrency::SerialTaskQueue>>
  createQueues(std::string const& resourceName) const;
  
  std::vector<std::shared_ptr<hep::concurrency::SerialTaskQueue>>
  createQueues(std::vector<std::string> const& resourceNames) const;
  
  std::recursive_mutex*
  getMutexForSource();
  
private: // MEMBER DATA

  std::map<std::string, QueueAndCounter>
  resourceMap_;

  unsigned
  nLegacy_;

  std::recursive_mutex
  mutexForSource_;

};

} // namespace art

#endif /* art_Framework_Core_SharedResourcesRegistry_h */

// Local Variables:
// mode: c++
// End:
