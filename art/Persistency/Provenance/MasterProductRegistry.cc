#include "art/Persistency/Provenance/MasterProductRegistry.h"
// vim: set sw=2 expandtab :

#include "cetlib/assert_only_one_thread.h"
#include "cetlib/container_algorithms.h"

using namespace std;

void
art::MasterProductRegistry::finalizeForProcessing(ProductTables const& productsToProduce)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  cet::for_all(productListUpdatedCallbacks_, [&productsToProduce](auto const& callback){ callback(productsToProduce); });
}

void
art::MasterProductRegistry::updateFromInputFile(ProductTables const& tables)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  cet::for_all(productListUpdatedCallbacks_, [&tables](auto const& callback){ callback(tables); });
}

void
art::MasterProductRegistry::registerProductListUpdatedCallback(ProductListUpdatedCallback cb)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  productListUpdatedCallbacks_.push_back(cb);
}
