#include "art/Framework/Services/Registry/detail/ServiceSetPool.h"

#include <algorithm>

namespace {
  std::queue<art::ServiceSetID>
  initialized_pool(size_t maxSize)
  {
    std::deque<art::ServiceSetID> tmp(maxSize);
    std::iota(tmp.begin(), tmp.end(), art::ServiceSetID());
    return std::queue<art::ServiceSetID>(std::move(tmp));
  }
}

art::detail::ServiceSetPool::
ServiceSetPool(size_t maxSize)
  :
  protectContents_(),
  queueNotEmpty_(),
  pool_(initialized_pool(maxSize)),
  inUse_()
{
}

art::ServiceSetID
art::detail::ServiceSetPool::
get()
{
  ServiceSetID result;
  ExecutionContext const & cc(ExecutionContextManager::top().baseContext());
  WaitLockType lock;
  lock.lock();
  if (!getCheckedOut(cc, result)) {
    result = checkOut(cc, lock);
  }
  lock.unlock();
  return result;
}

bool
art::detail::ServiceSetPool::
try_get(ServiceSetID &id)
{
  ExecutionContext const & cc(ExecutionContextManager::top().baseContext());
  WaitLockType lock;
  if (!lock.try_lock()) {
    return false;
  }
  if (!getCheckedOut(cc, id)) {
    id = checkOut(cc, lock);
  }
  lock.unlock();
  return true;
}

void
art::detail::ServiceSetPool::
release(ExecutionContext const & context)
{
  LockType lock(protectContents_);
  auto it(findContext_(context));
  if (--(it->count) == 0) {
    pool_.push(it->ssid);
    inUse_.erase(it);
    queueNotEmpty_.notify_one();
  }
}

// Lock assumed obtained by calling context.
bool
art::detail::ServiceSetPool::
getCheckedOut(ExecutionContext const & context,
              ServiceSetID & ssid)
{
  auto it(findContext_(context));
  if (it != inUse_.cend()) {
    ++it->count;
    ssid = it->ssid;
    return true;
  }
  return false;
}

// Lock assumed obtained by calling context.
art::ServiceSetID
art::detail::ServiceSetPool::
checkOut(ExecutionContext const & context,
         WaitLockType & lock)
{
  while (pool_.empty()) {
    queueNotEmpty_.wait(lock);
  }
  ServiceSetID result(pool_.front());
  pool_.pop();
  inUse_.emplace_front(context, result, 0);
  return result;
}

// inUse assumed locked by calling context.
auto
art::detail::ServiceSetPool::
findContext_(ExecutionContext const & context)
-> InUseCache::iterator
{
  return
    std::find_if(inUse_.begin(),
                 inUse_.end(),
                 [&context](InUseItem const & item) {
                   return compareContexts(context, item.context);
                 });
}

