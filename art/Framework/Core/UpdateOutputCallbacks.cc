#include "art/Framework/Core/UpdateOutputCallbacks.h"
// vim: set sw=2 expandtab :

#include "cetlib/container_algorithms.h"
#include "hep_concurrency/assert_only_one_thread.h"

using namespace std;

void
art::UpdateOutputCallbacks::invoke(ProductTables const& tables)
{
  HEP_CONCURRENCY_ASSERT_ONLY_ONE_THREAD();
  cet::for_all(callbacks_,
               [&tables](auto const& callback) { callback(tables); });
}

void
art::UpdateOutputCallbacks::registerCallback(ProductListUpdatedCallback cb)
{
  HEP_CONCURRENCY_ASSERT_ONLY_ONE_THREAD();
  callbacks_.push_back(cb);
}
