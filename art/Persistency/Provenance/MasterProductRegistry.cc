#include "art/Persistency/Provenance/MasterProductRegistry.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchKey.h"
#include "cetlib/assert_only_one_thread.h"
#include "cetlib/exception.h"

#include <cassert>
#include <ostream>

using namespace std;

void
art::MasterProductRegistry::finalizeForProcessing()
{
  CET_ASSERT_ONLY_ONE_THREAD();
  // Product registration can still happen implicitly whenever an
  // input file is opened--via calls to updateFromInputFile.
  allowExplicitRegistration_ = false;
  cet::for_all(productListUpdatedCallbacks_, [this](auto const& callback){ callback(productLists_); });
}

void
art::MasterProductRegistry::addProductsFromModule(ProductDescriptions&& descriptions)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  for (auto&& desc : descriptions) {
    addProduct_(std::move(desc));
  }
}

void
art::MasterProductRegistry::updateFromModule(std::unique_ptr<ProductLists>&& pl)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  if (!pl) return;
  updateProductLists_(*pl);
}

void
art::MasterProductRegistry::updateFromInputFile(ProductLists const& pl)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  updateProductLists_(pl);
  cet::for_all(productListUpdatedCallbacks_, [&pl](auto const& callback){ callback(pl); });
}

void
art::MasterProductRegistry::registerProductListUpdatedCallback(ProductListUpdatedCallback cb)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  productListUpdatedCallbacks_.push_back(cb);
}

void
art::MasterProductRegistry::print(std::ostream& os) const
{
  // TODO: Shouldn't we print the BranchKey too?
  for (auto const& list : productLists_) {
    for (auto const& val : list ) {
      os << val.second << "\n-----\n";
    }
  }
}

//=====================================================================================
// Private member functions

void
art::MasterProductRegistry::addProduct_(BranchDescription&& bdp)
{
  CET_ASSERT_ONLY_ONE_THREAD();

  // The below check exists primarily to ensure that the framework
  // does not accidentally call addProduct at a time when it should
  // not.
  if (!allowExplicitRegistration_) {
    throw Exception(errors::ProductRegistrationFailure)
      << "An attempt to register the product\n"
      << bdp
      << "was made after the product registry was frozen.\n"
      << "Product registration can be done only in module constructors.\n";
  }

  // assert(bdp.produced());

  auto it = productLists_[bdp.branchType()].emplace(bdp.productID(), BranchDescription{});
  if (!it.second) {
    throw Exception(errors::Configuration)
      << "The process name "
      << bdp.processName()
      << " was previously used on these products.\n"
      << "Please modify the configuration file to use a "
      << "distinct process name.\n";
  }
  auto& productListEntry = *it.first;
  auto& pd = productListEntry.second;
  pd.swap(bdp);
  productProduced_[pd.branchType()] = true;
}

void
art::MasterProductRegistry::updateProductLists_(ProductDescriptionsByID const& pl)
{
  for (auto const& val : pl) {
    auto const& pd = val.second;
    assert(!pd.produced());
    auto& productList = productLists_[pd.branchType()];
    auto const pid = pd.productID();
    auto it = productList.find(pid);
    if (it == productList.end()) {
      // New product
      productList.emplace(pid, pd);
      continue;
    }
    auto& found_pd = it->second;
    assert(combinable(found_pd, pd));
    found_pd.merge(pd);
  }
}

void
art::MasterProductRegistry::updateProductLists_(ProductLists const& lists)
{
  for (auto const& list : lists) {
    updateProductLists_(list);
  }
}

std::ostream&
art::operator<<(std::ostream& os, MasterProductRegistry const& mpr)
{
  mpr.print(os);
  return os;
}
