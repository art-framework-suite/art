#include "art/Persistency/Provenance/MasterProductRegistry.h"
// vim: set sw=2:

#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Persistency/Provenance/TypeWithDict.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/exception.h"

#include <cassert>
#include <ostream>
#include <sstream>

using namespace art;

namespace {

  struct PendingBTLEntry {
    PendingBTLEntry(BranchType bt, std::string fcn, std::string procName,
                    std::string moduleLabel, std::string instanceName,
                    BranchID bid)
      : bk(std::move(fcn), std::move(moduleLabel), std::move(instanceName), std::move(procName), bt)
      , bid(bid)
      {}

    BranchKey bk;
    BranchID bid;
  };

  void
  recreateLookups(ProductList const& prods,
                  BranchTypeLookup& pl,
                  BranchTypeLookup& el)
  {
    std::vector<PendingBTLEntry> pendingEntries;
    std::unordered_map<BranchID,
      std::pair<std::string, std::string>,
      BranchID::Hash> insertedABVs;
    for (auto const& val: prods) {
      auto const& procName = val.first.processName_;
      auto const& bid = val.second.branchID();
      auto const& prodFCN = val.first.friendlyClassName_;
      auto const bt = val.first.branchType_;
      pl[bt][prodFCN][procName].emplace_back(bid);
      // Look in the class of the product for a typedef named "value_type",
      // if there is one allow lookups by that type name too (and by all
      // of its base class names as well).
      art::TypeWithDict const TY {val.second.producedClassName()};
      if (TY.category() != art::TypeWithDict::Category::CLASSTYPE) {
        continue;
      }
      TClass* TYc = TY.tClass();
      auto ET = mapped_type_of(TYc);
      if (ET || (ET = value_type_of(TYc))) {
        // The class of the product has a nested type, "mapped_type," or,
        // "value_type," so allow lookups by that type and all of its base
        // types too.
        auto vtFCN = ET.friendlyClassName();
        el[bt][vtFCN][procName].emplace_back(bid);
        if (ET.category() == art::TypeWithDict::Category::CLASSTYPE) {
          // Repeat this for all public base classes of the value_type.
          std::vector<TClass*> bases;
          art::public_base_classes(ET.tClass(), bases);
          for (auto BT: bases) {
            auto btFCN = art::TypeID{BT->GetTypeInfo()}.friendlyClassName();
            el[bt][btFCN][procName].emplace_back(bid);
          }
        }
      }
      if (is_instantiation_of(TYc, "art::Assns")) {
        auto const base = find_nested_type_named("base", TYc);
        if (base) { // We're an Assns<A, B, D>, with a base Assns<A, B>.
          // Add this to the list of "second-tier" products to register
          // later.
          assert(base.category() == art::TypeWithDict::Category::CLASSTYPE);
          auto baseFCN = base.friendlyClassName();
          pendingEntries.emplace_back(BranchType(bt), baseFCN, procName,
                                      val.first.moduleLabel_,
                                      val.first.productInstanceName_,
                                      bid);
        } else {
          // Add our bid to the list of real Assns<A, B, void> products
          // already registered.
          insertedABVs[bid] =
            std::make_pair(val.first.moduleLabel_,
                           val.first.productInstanceName_);
        }
      }
    }
    auto const iend = insertedABVs.cend();
    // Preserve useful ordering, only inserting if we don't already have
    // a *real* Assns<A, B, void> for that module label / instance name
    // combination.
    std::for_each(pendingEntries.cbegin(),
                  pendingEntries.cend(),
                  [&pl, &insertedABVs, iend](auto const & pe)
                  {
                    auto & bids = pl[pe.bk.branchType_][pe.bk.friendlyClassName_][pe.bk.processName_];
                    if (bids.empty() ||
                        !std::any_of(bids.cbegin(), bids.cend(),
                                     [&insertedABVs, &iend, &pe](BranchID const & bid) {
                                       auto i = insertedABVs.find(bid);
                                       return i != iend &&
                                         i->second.first == pe.bk.moduleLabel_ &&
                                         i->second.second == pe.bk.productInstanceName_;
                                     }))
                    {
                      bids.emplace_back(pe.bid);
                    }
                  });
  }
}

namespace art {

  MasterProductRegistry::
  MasterProductRegistry()
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
    checkDicts_(*bdp);
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
    perFileProds_[0].insert(productListEntry);
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
    for (auto const& p : pl) {
      auto const& bd = p.second;
      auto const& presListForBT = presList[bd.branchType()];
      if (presListForBT.find(bd.branchID()) != presListForBT.cend()) {
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
      checkDicts_(bd);
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
    for (auto const& p : pl) {
      auto const & bd = p.second;
      auto const & presListForBT = presList[bd.branchType()];
      if (presListForBT.find(bd.branchID()) != presListForBT.cend()) {
        perFilePresenceLookups_.back()[bd.branchType()].emplace(bd.branchID());
      }
    }

    for (auto const& val: pl) {
      auto const& bd = val.second;
      assert(!bd.produced());
      checkDicts_(bd);
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
    for (auto const& p : other) {
      auto const& bd = p.second;
      auto const& presListForBT = presList[bd.branchType()];
      if (presListForBT.find(bd.branchID()) != presListForBT.cend()) {
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
        checkDicts_(J->second);
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
    for (std::size_t i{}; i != perFilePresenceLookups_.size() ; ++i) {
      auto& pLookup = perFilePresenceLookups_[i][branchType];
      if (pLookup.find(bid) != pLookup.cend())
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

  void
  MasterProductRegistry::
  checkDicts_(BranchDescription const& productDesc)
  {
    auto const isTransient = productDesc.transient();

    // Check product dictionaries.
    dictChecker_.checkDictionaries(productDesc.wrappedName(), false);
    dictChecker_.checkDictionaries(productDesc.producedClassName(), !isTransient);

    // Check dictionaries for assnsPartner, if appropriate. This is only
    // necessary for top-level checks so appropriate here rather than
    // checkDictionaries itself.
    if (!isTransient) {
      auto const assnsPartner =
        name_of_assns_partner(productDesc.producedClassName());
      if (!assnsPartner.empty()) {
        dictChecker_.checkDictionaries(wrappedClassName(assnsPartner), true);
      }
    }
    dictChecker_.reportMissingDictionaries();
  }

  std::ostream&
  operator<<(std::ostream& os, MasterProductRegistry const& mpr)
  {
    mpr.print(os);
    return os;
  }

} // namespace art
