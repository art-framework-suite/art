#include "art/Persistency/Provenance/MasterProductRegistry.h"
// vim: set sw=2:

#include "Reflex/Type.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ReflexTools.h"
#include "art/Utilities/TypeID.h"
#include "art/Utilities/WrappedClassName.h"
#include "cetlib/exception.h"
#include <cassert>
#include <ostream>
#include <sstream>

namespace art {

// FIXME: The results are only printed out when freezing, and
// FIXME: so we do not notice problems with the second and
// FIXME: following primary files, nor with secondary files.
static
void
checkDicts(art::BranchDescription const& productDesc)
{
  if (productDesc.transient()) {
    art::checkDictionaries(productDesc.producedClassName(), true);
  }
  else {
    art::checkDictionaries(
      art::wrappedClassName(productDesc.producedClassName()), false);
  }
}

static
void
recreateLookups(ProductList const& prods,
                MasterProductRegistry::TypeLookup& pl,
                MasterProductRegistry::TypeLookup& el)
{
  for (auto const& val: prods) {
    auto const& procName = val.first.processName_;
    auto const& bid = val.second.branchID();
    auto const& prodFCN = val.first.friendlyClassName_;
    pl[prodFCN][procName].push_back(bid);
    // Look in the class of the product for a typedef named "value_type",
    // if there is one allow lookups by that type name too (and by all
    // of its base class names as well).
    Reflex::Type TY(Reflex::Type::ByName(val.second.producedClassName()));
    if (!TY) {
      // We do not have a dictionary for the class of the product.
      continue;
    }
    Reflex::Type VT;
    if (value_type_of(TY, VT) && VT) {
      // The class of the product does have a member type named "value_type",
      // so allow lookups by that type and all of its base types too.
      auto vtFCN = TypeID(VT.TypeInfo()).friendlyClassName();
      el[vtFCN][procName].push_back(bid);
      // Repeat this for all public base classes of the value_type.
      std::vector<Reflex::Type> bases;
      public_base_classes(VT, bases);
      for (auto const& BT: bases) {
        auto btFCN = TypeID(BT.TypeInfo()).friendlyClassName();
        el[btFCN][procName].push_back(bid);
      }
    }
  }
}

MasterProductRegistry::
MasterProductRegistry()
  : productList_()
  , frozen_(false)
  , productProduced_()
  , perFileProds_()
  , productLookup_()
  , elementLookup_()
{
  productProduced_.fill(false);
  perFileProds_.resize(1);
}

void
MasterProductRegistry::
addProduct(std::unique_ptr<BranchDescription>&& bdp)
{
  assert(bdp->produced());
  if (frozen_) {
    throw cet::exception("ProductRegistry", "addProduct")
        << "Cannot modify the MasterProductRegistry because it is frozen.\n";
  }
  checkDicts(*bdp);
  auto I = productList_.emplace(BranchKey(*bdp), BranchDescription());
  if (!I.second) {
    throw Exception(errors::Configuration)
        << "The process name "
        << bdp->processName()
        << " was previously used on these products.\n"
        << "Please modify the configuration file to use a "
        << "distinct process name.\n";
  }
  I.first->second.swap(*bdp);
  perFileProds_[0].insert(*I.first);
  productProduced_[I.first->second.branchType()] = true;
}

void
MasterProductRegistry::
initFromFirstPrimaryFile(ProductList const& pl, FileBlock const& fb)
{
  for (auto const& val: pl) {
    auto const& bd = val.second;
    assert(!bd.produced());
    if (frozen_) {
      throw cet::exception("ProductRegistry", "initFromFirstPrimaryFile")
          << "Cannot modify the MasterProductRegistry because it is frozen.\n";
    }
    checkDicts(bd);
    auto bk = BranchKey(bd);
    auto I = productList_.find(bk);
    if (I == productList_.end()) {
      // New product.
      productList_.emplace(bk, bd);
      perFileProds_[0].emplace(bk, bd);
      continue;
    }
    // Already had this product, combine in the additional parameter
    // sets and process descriptions.
    assert(combinable(I->second, bd));
    I->second.merge(bd);
    auto J = perFileProds_[0].find(bk);
    assert(J != perFileProds_[0].end());
    assert(combinable(J->second, bd));
    J->second.merge(bd);
  }
  for (auto const& val : productListUpdatedCallbacks_) {
    (*val.first)(val.second.get(), fb);
  }
}

void
MasterProductRegistry::
updateFromSecondaryFile(ProductList const& pl, FileBlock const& fb)
{
  perFileProds_.resize(perFileProds_.size()+1);
  for (auto const& val: pl) {
    auto const& bd = val.second;
    assert(!bd.produced());
    checkDicts(bd);
    auto bk = BranchKey(bd);
    auto I = productList_.find(bk);
    if (I == productList_.end()) {
      // New product.
      productList_.emplace(bk, bd);
      perFileProds_.back().emplace(bk, bd);
      continue;
    }
    // Already had this product, combine in the additional parameter
    // sets and process descriptions.
    assert(combinable(I->second, bd));
    I->second.merge(bd);
    // Now repeat for the per-file prods list.
    auto J = perFileProds_.back().find(bk);
    if (J == perFileProds_.back().end()) {
      // New product.
      perFileProds_.back().emplace(bk, bd);
      continue;
    }
    // Already had this product, combine in the additional parameter
    // sets and process descriptions.
    assert(combinable(J->second, bd));
    J->second.merge(bd);
  }
  productLookup_.resize(productLookup_.size()+1);
  elementLookup_.resize(elementLookup_.size()+1);
  recreateLookups(perFileProds_.back(), productLookup_.back(),
                  elementLookup_.back());
  for (auto const& val : productListUpdatedCallbacks_) {
    (*val.first)(val.second.get(), fb);
  }
}

std::string
MasterProductRegistry::
updateFromNewPrimaryFile(ProductList const& other, std::string const& fileName,
                         BranchDescription::MatchMode m, FileBlock const& fb)
{
  perFileProds_.resize(1);
  std::ostringstream msg;
  auto I = productList_.begin();
  auto E = productList_.end();
  auto J = other.cbegin();
  auto JE = other.cend();
  // Loop over entries in the main product registry.
  while ((I != E) || (J != JE)) {
    if ((I != E) && I->second.produced()) {
      // Skip branches produced by the current process.
      ++I;
      continue;
    }
    if ((I == E) || ((J != JE) && (J->first < I->first))) {
      // We have found a product listed in the new input file
      // product list which was not in the product list of any
      // previous input file.
      assert(!J->second.produced());
      if (!J->second.present()) {
        // FIXME: This probably cannot happen because of
        // FIXME: the way dropping is done by the output!
        // Allow it if it was dropped from the new input file.
        productList_.insert(*J);
        perFileProds_[0].insert(*J);
      }
      else {
        // Complain, the new input file is trying to introduce
        // a product that none of the previous input files know
        // anything about.
        msg << "Branch '"
            << J->second.branchName()
            << "' is in file '"
            << fileName
            << "'\n"
            << "    but not in previous files.\n";
      }
      ++J;
      continue;
    }
    if ((J == JE) || ((I != E) && (I->first < J->first))) {
      // We have found a product which was listed in at least
      // one of the previous input files product lists but
      // is not listed in the product list of the new file.
      // This is ok, products are allowed to be dropped.
      ++I;
      continue;
    }
    assert(!J->second.produced());
    // Check if the product listed in the new file matches the
    // product at the current position in our product list.
    std::string err = match(I->second, J->second, fileName, m);
    if (!err.empty()) {
      // Did not match, complain, and skip it.
      msg << err;
    }
    else if (m == BranchDescription::Permissive) {
      // We had a match and we are permitting a branch description merge.
      auto const& bd = J->second;
      I->second.merge(bd);
    }
    ++I;
    ++J;
  }
  productLookup_.clear();
  productLookup_.resize(1);
  elementLookup_.clear();
  elementLookup_.resize(1);
  recreateLookups(productList_, productLookup_[0], elementLookup_[0]);
  for (auto const& val : productListUpdatedCallbacks_) {
    (*val.first)(val.second.get(), fb);
  }
  return msg.str();
}

void
MasterProductRegistry::
setFrozen()
{
  reportFailedDictionaryChecks();
  if (frozen_) {
    return;
  }
  frozen_ = true;
  productLookup_.clear();
  productLookup_.resize(1);
  elementLookup_.clear();
  elementLookup_.resize(1);
  recreateLookups(productList_, productLookup_[0], elementLookup_[0]);
}

void
MasterProductRegistry::
print(std::ostream& os) const
{
  // TODO: Shouldn't we print the BranchKey too?
  for (auto const& val: productList_) {
    os << val.second << "\n-----\n";
  }
}

std::ostream&
operator<<(std::ostream& os, MasterProductRegistry const& mpr)
{
  mpr.print(os);
  return os;
}

} // namespace art

