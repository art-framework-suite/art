#include "art/Framework/Core/UpdateOutputCallbacks.h"
// vim: set sw=2 expandtab :

#include "cetlib/assert_only_one_thread.h"
#include "cetlib/container_algorithms.h"

using namespace std;

void
art::UpdateOutputCallbacks::invoke(ProductTables const& tables)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  cet::for_all(callbacks_,
               [&tables](auto const& callback) { callback(tables); });
}

void
art::UpdateOutputCallbacks::registerCallback(ProductListUpdatedCallback cb)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  callbacks_.push_back(cb);
}
