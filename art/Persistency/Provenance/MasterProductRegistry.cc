#include "art/Persistency/Provenance/MasterProductRegistry.h"
// vim: set sw=2:

#include "canvas/Persistency/Provenance/BranchKey.h"
#include "cetlib/assert_only_one_thread.h"
#include "cetlib/exception.h"

#include <cassert>
#include <ostream>

using namespace art;

void
art::MasterProductRegistry::addProduct(std::unique_ptr<BranchDescription>&& bdp)
{
  CET_ASSERT_ONLY_ONE_THREAD();

  // The below check exists primarily to ensure that the framework
  // does not accidentally call addProduct at a time when it should
  // not.
  if (!allowExplicitRegistration_) {
    throw Exception(errors::ProductRegistrationFailure)
      << "An attempt to register the product\n"
      << *bdp
      << "was made after the product registry was frozen.\n"
      << "Product registration can be done only in module constructors.\n";
  }

  assert(bdp->produced());

  auto it = productList_.emplace(BranchKey{*bdp}, BranchDescription{});
  if (!it.second) {
    throw Exception(errors::Configuration)
      << "The process name "
      << bdp->processName()
      << " was previously used on these products.\n"
      << "Please modify the configuration file to use a "
      << "distinct process name.\n";
  }
  auto& productListEntry = *it.first;
  auto& pd = productListEntry.second;
  pd.swap(*bdp);
  productProduced_[pd.branchType()] = true;
  perBranchPresenceLookup_[pd.branchType()].emplace(pd.productID());
}

void
art::MasterProductRegistry::finalizeForProcessing()
{
  CET_ASSERT_ONLY_ONE_THREAD();
  // Product registration can still happen implicitly whenever an
  // input file is opened--via calls to updateFrom(Primary|Secondary)File.
  allowExplicitRegistration_ = false;
}

void
art::MasterProductRegistry::updateFromModule(std::unique_ptr<ProductList>&& pl)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  if (!pl) return;

  updateProductLists_(*pl);
}

void
art::MasterProductRegistry::updateFromPrimaryFile(ProductList const& pl,
                                                  PerBranchTypePresence const& presList,
                                                  FileBlock const& fb)
{
  CET_ASSERT_ONLY_ONE_THREAD();

  updateProductLists_(pl);
  perFilePresenceLookups_.assign(1u, presList);

  cet::for_all(productListUpdatedCallbacks_, [&fb](auto const& callback){ callback(fb); });
}

void
art::MasterProductRegistry::updateFromSecondaryFile(ProductList const& pl,
                                                    PerBranchTypePresence const& presList,
                                                    FileBlock const& fb)
{
  CET_ASSERT_ONLY_ONE_THREAD();

  updateProductLists_(pl);
  perFilePresenceLookups_.push_back(presList);

  cet::for_all(productListUpdatedCallbacks_, [&fb](auto const& callback){ callback(fb); });
}

void
art::MasterProductRegistry::registerProductListUpdatedCallback(ProductListUpdatedCallback cb)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  productListUpdatedCallbacks_.push_back(cb);
}

bool
art::MasterProductRegistry::produced(BranchType const branchType, ProductID const pid) const
{
  auto const& pLookup = perBranchPresenceLookup_[branchType];
  return pLookup.find(pid) != pLookup.cend();
}

std::size_t
art::MasterProductRegistry::presentWithFileIdx(BranchType const branchType, ProductID const pid) const
{
  for (std::size_t i{}; i != perFilePresenceLookups_.size() ; ++i) {
    auto& pLookup = perFilePresenceLookups_[i][branchType];
    if (pLookup.find(pid) != pLookup.cend())
      return i;
  }
  return DROPPED;
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
art::MasterProductRegistry::updateProductLists_(ProductList const& pl)
{
  for (auto const& val: pl) {
    auto const& pd = val.second;
    assert(!pd.produced());
    auto bk = BranchKey{pd};
    auto it = productList_.find(bk);
    if (it == productList_.end()) {
      // Product from input file is not in the master product list.
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
