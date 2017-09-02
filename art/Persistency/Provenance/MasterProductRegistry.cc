#include "art/Persistency/Provenance/MasterProductRegistry.h"
// vim: set sw=2:

#include "canvas/Persistency/Provenance/BranchKey.h"
#include "cetlib/assert_only_one_thread.h"
#include "cetlib/exception.h"

#include <cassert>
#include <ostream>

using namespace art;

void
art::MasterProductRegistry::finalizeForProcessing()
{
  CET_ASSERT_ONLY_ONE_THREAD();
  // Product registration can still happen implicitly whenever an
  // input file is opened--via calls to updateFromInputFile.
  allowExplicitRegistration_ = false;
  cet::for_all(productListUpdatedCallbacks_, [this](auto const& callback){ callback(productList_); });
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
art::MasterProductRegistry::updateFromModule(std::unique_ptr<ProductList>&& pl)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  if (!pl) return;
  updateProductLists_(*pl);
}

void
art::MasterProductRegistry::updateFromInputFile(ProductList const& pl)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  updateProductLists_(pl);
  cet::for_all(productListUpdatedCallbacks_, [this](auto const& callback){ callback(productList_); });
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
  for (auto const& val: productList_) {
    os << val.second << "\n-----\n";
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

  assert(bdp.produced());

  auto it = productList_.emplace(BranchKey{bdp}, BranchDescription{});
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
art::MasterProductRegistry::updateProductLists_(ProductList const& pl)
{
  for (auto const& val: pl) {
    auto const& pd = val.second;
    assert(!pd.produced());
    auto bk = BranchKey{pd};
    auto it = productList_.find(bk);
    if (it == productList_.end()) {
      // New product
      productList_.emplace(bk, pd);
      continue;
    }
    auto& found_pd = it->second;
    assert(combinable(found_pd, pd));
    found_pd.merge(pd);
  }
}

std::ostream&
art::operator<<(std::ostream& os, MasterProductRegistry const& mpr)
{
  mpr.print(os);
  return os;
}
