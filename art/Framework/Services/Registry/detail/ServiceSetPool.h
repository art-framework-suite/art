#ifndef art_Framework_Services_Registry_detail_ServiceSetPool_h
#define art_Framework_Services_Registry_detail_ServiceSetPool_h

#include "art/Persistency/Provenance/ExecutionContext.h"
#include "art/Persistency/Provenance/ExecutionContextManager.h"
#include "art/Utilities/ServiceSetID.h"

#include <condition_variable>
#include <deque>
#include <map>
#include <mutex>
#include <queue>

namespace art {
  namespace detail {
    class ServiceSetPool;
  }
}

class art::detail::ServiceSetPool {
public:
  ServiceSetPool(size_t maxSize);

  ServiceSetID get();
  bool try_get(ServiceSetID &id);
  void release(ExecutionContext const & context);

private:
  typedef std::lock_guard<std::mutex> LockType;
  typedef std::unique_lock<std::mutex> WaitLockType;
  struct InUseItem {
    InUseItem(ExecutionContext const & context,
              ServiceSetID ssid,
              size_t count);
    ExecutionContext context;
    ServiceSetID ssid;
    size_t count;
  };
  typedef std::deque<InUseItem> InUseCache;

  bool getCheckedOut(ExecutionContext const & context,
                     ServiceSetID & ssid);
  ServiceSetID checkOut(ExecutionContext const & context,
                        WaitLockType & lock);
  InUseCache::iterator
  findContext_(ExecutionContext const & context);

  mutable std::mutex protectContents_;
  mutable std::condition_variable queueNotEmpty_;

  std::queue<ServiceSetID> pool_;
  InUseCache inUse_;
};

inline
art::detail::ServiceSetPool::InUseItem::
InUseItem(ExecutionContext const & context,
          ServiceSetID ssid,
          size_t count)
:
  context(context),
  ssid(ssid),
  count(count)
{
}

#endif /* art_Framework_Services_Registry_detail_ServiceSetPool_h */

// Local Variables:
// mode: c++
// End:
