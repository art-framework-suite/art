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

namespace {

  using namespace art;

  void
  checkDicts(BranchDescription const& productDesc)
  {
    if (productDesc.transient()) {
      checkDictionaries(productDesc.wrappedName(), true);
      checkDictionaries(productDesc.producedClassName(), true);
    }
    else {
      checkDictionaries(productDesc.wrappedName(), false);
    }
    reportFailedDictionaryChecks();
  }

  void
  recreateLookups(ProductList const& prods,
                  BranchTypeLookup& pl,
                  BranchTypeLookup& el)
  {
    for (auto const& val: prods) {
      auto const& procName = val.first.processName_;
      auto const& bid = val.second.branchID();
      auto const& prodFCN = val.first.friendlyClassName_;
      auto const bt = val.first.branchType_;
      pl[bt][prodFCN][procName].push_back(bid);
      // Look in the class of the product for a typedef named "value_type",
      // if there is one allow lookups by that type name too (and by all
      // of its base class names as well).
      Reflex::Type TY(Reflex::Type::ByName(val.second.producedClassName()));
      if (!TY) {
        // We do not have a dictionary for the class of the product.
        continue;
      }
      Reflex::Type ET;
      if ((mapped_type_of(TY, ET) || value_type_of(TY, ET)) && ET) {
        // The class of the product has a nested type, "mapped_type," or,
        // "value_type," so allow lookups by that type and all of its base
        // types too.
        auto vtFCN = TypeID(ET.TypeInfo()).friendlyClassName();
        el[bt][vtFCN][procName].push_back(bid);
        // Repeat this for all public base classes of the value_type.
        std::vector<Reflex::Type> bases;
        public_base_classes(ET, bases);
        for (auto const& BT: bases) {
          auto btFCN = TypeID(BT.TypeInfo()).friendlyClassName();
          el[bt][btFCN][procName].push_back(bid);
        }
      }
    }
  }
}

namespace art {

  MasterProductRegistry::
  MasterProductRegistry()
    : productList_()
    , frozen_(false)
    , productProduced_{false}
    , perFileProds_()
    , perFilePresenceLookups_()
    , productLookup_()
    , elementLookup_()
    {
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
    auto & productListEntry = *I.first;
    auto & bd = productListEntry.second;
    bd.swap(*bdp);
    perFileProds_[0].insert( productListEntry );
    productProduced_[bd.branchType()] = true;
    perBranchPresenceLookup_[bd.branchType()].emplace(bd.branchID());
  }

  void
  MasterProductRegistry::
  initFromFirstPrimaryFile(ProductList const& pl,
                           PerBranchTypePresence const& presList,
                           FileBlock const& fb)
  {
    perFilePresenceLookups_.resize(1);

    // Set presence flags
    for (auto const& p : pl ) {
      auto const & bd = p.second;
      auto const & presListForBT = presList[bd.branchType()];
      if ( presListForBT.find( bd.branchID() ) != presListForBT.cend() ) {
        perFilePresenceLookups_[0][bd.branchType()].emplace(bd.branchID());
      }
    }

    // Set product lists and handle merging
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
      //    assert(false);
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
      val(fb);
    }
  }

  void
  MasterProductRegistry::
  updateFromSecondaryFile(ProductList const& pl,
                          PerBranchTypePresence const& presList,
                          FileBlock const& fb)
  {
    perFileProds_.resize(perFileProds_.size()+1);
    perFilePresenceLookups_.resize(perFilePresenceLookups_.size()+1);

    // Set presence flags
    for (auto const& p : pl ) {
      auto const & bd = p.second;
      auto const & presListForBT = presList[bd.branchType()];
      if ( presListForBT.find( bd.branchID() ) != presListForBT.cend() ) {
        perFilePresenceLookups_.back()[bd.branchType()].emplace(bd.branchID());
      }
    }

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
      val(fb);
    }
  }

  std::string
  MasterProductRegistry::
  updateFromNewPrimaryFile(ProductList const& other,
                           PerBranchTypePresence const& presList,
                           std::string const& fileName,
                           BranchDescription::MatchMode m, FileBlock const& fb)
  {
    perFileProds_.resize(1);

    perFilePresenceLookups_.clear();
    perFilePresenceLookups_.resize(1);

    // Set presence flags
    for (auto const& p : other ) {
      auto const & bd = p.second;
      auto const & presListForBT = presList[bd.branchType()];
      if ( presListForBT.find( bd.branchID() ) != presListForBT.cend() ) {
        perFilePresenceLookups_[0][bd.branchType()].emplace(bd.branchID());
      }
    }

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
        checkDicts(J->second);
        productList_.insert(*J);
        perFileProds_[0].insert(*J);
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
      val(fb);
    }
    return msg.str();
  }

  bool
  MasterProductRegistry::
  produced(BranchType const branchType, BranchID const bid) const
  {
    auto const& pLookup = perBranchPresenceLookup_[branchType];
    return pLookup.find(bid) != pLookup.cend();
  }

  std::size_t
  MasterProductRegistry::
  presentWithFileIdx(BranchType const branchType, BranchID const bid) const
  {
    for ( std::size_t i = 0; i != perFilePresenceLookups_.size() ; ++i) {
      auto & pLookup = perFilePresenceLookups_[i][branchType];
      if ( pLookup.find(bid) != pLookup.cend() )
        return i;
    }
    return DROPPED;
  }

  void
  MasterProductRegistry::
  setFrozen()
  {
    if (frozen_) {
      return;
    }
    reportFailedDictionaryChecks();
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
