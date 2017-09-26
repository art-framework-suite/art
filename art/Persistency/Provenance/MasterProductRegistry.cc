#include "art/Persistency/Provenance/MasterProductRegistry.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchKey.h"
#include "cetlib/assert_only_one_thread.h"
#include "cetlib/exception.h"

#include <cassert>
#include <ostream>

using namespace std;

void
art::MasterProductRegistry::finalizeForProcessing(ProductTables const& productsToProduce)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  // Product registration can still happen implicitly whenever an
  // input file is opened--via calls to updateFromInputFile.
  allowExplicitRegistration_ = false;
  cet::for_all(productListUpdatedCallbacks_, [&productsToProduce](auto const& callback){ callback(productsToProduce); });
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
art::MasterProductRegistry::updateFromModule(std::unique_ptr<ProductTables>&& tables)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  if (!tables) return;
  updateProductTables_(*tables);
}

void
art::MasterProductRegistry::updateFromInputFile(ProductTables const& tables)
{
  CET_ASSERT_ONLY_ONE_THREAD();
  updateProductTables_(tables);
  cet::for_all(productListUpdatedCallbacks_, [&tables](auto const& callback){ callback(tables); });
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
  auto print_products = [this, &os](BranchType const bt) {
    for (auto const& val : productTables_.descriptions(bt)) {
      os << val.second << "\n-----\n";
    }
  };
  for_each_branch_type(print_products);
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

  auto& descriptions = productTables_.get(bdp.branchType()).descriptions;
  auto it = descriptions.emplace(bdp.productID(), bdp);
  if (it.second) {
    productProduced_[it.first->second.branchType()] = true;
    return;
  }

  // The 'combinable' call does not require that the processing
  // history be the same, which is not what we are checking for here.
  if (combinable(it.first->second, bdp)) {
    throw Exception(errors::Configuration)
      << "The process name "
      << bdp.processName()
      << " was previously used on these products.\n"
      << "Please modify the configuration file to use a "
      << "distinct process name.\n";
  }

  throw Exception(errors::ProductRegistrationFailure)
    << "The product ID " << bdp.productID()
    << " of the new product:\n"
    << bdp
    << " collides with the product ID of the already-existing product:\n"
    << it.first->second
    << "Please modify the instance name of the new product so as to avoid the product ID collision.\n"
    << "In addition, please notify artists@fnal.gov of this error.\n";
}

void
art::MasterProductRegistry::updateProductTables_(ProductDescriptionsByID const& pl)
{
  for (auto const& val : pl) {
    auto const& pd = val.second;
    assert(!pd.produced());
    auto& productList = productTables_.get(pd.branchType()).descriptions;
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
art::MasterProductRegistry::updateProductTables_(ProductTables const& tables)
{
  for_each_branch_type([this, &tables](BranchType const bt){ this->updateProductTables_(tables.descriptions(bt)); });
}

std::ostream&
art::operator<<(std::ostream& os, MasterProductRegistry const& mpr)
{
  mpr.print(os);
  return os;
}
