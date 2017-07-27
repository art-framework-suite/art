#include "art/Persistency/Provenance/MasterProductRegistry.h"
// vim: set sw=2:

#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Persistency/Provenance/TypeWithDict.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/assert_only_one_thread.h"
#include "cetlib/exception.h"

#include <cassert>
#include <map>
#include <ostream>
#include <sstream>
#include <unordered_map>

using namespace art;

namespace {

  class CheapTag {
  public:
    CheapTag(std::string const& label,
             std::string const& instance,
             std::string const& process)
      : label_{label}
      , instance_{instance}
      , process_{process}
      {}

    std::string const& label() const { return label_;}
    std::string const& instance() const { return instance_; }
    std::string const& process() const { return process_; }

  private:
    std::string label_;
    std::string instance_;
    std::string process_;
  };

  inline
  bool
  operator==(CheapTag const& left, CheapTag const& right)
  {
    return left.label() == right.label() &&
      left.instance() == right.instance() &&
      left.process() == right.process();
  }

  inline
  bool
  operator!=(CheapTag const& left, CheapTag const& right)
  {
    return !(left == right);
  }

  class PendingBTLEntry {
  public:
    PendingBTLEntry(BranchType const bt,
                    std::string const& fcn,
                    std::string const& moduleLabel,
                    std::string const& instanceName,
                    std::string const& procName,
                    ProductID const pid)
      : bt_{bt}
      , fcn_{fcn}
      , ct_{moduleLabel, instanceName, procName}
      , pid_{pid}
    {}

    BranchType bt() const { return bt_; }
    std::string const& fcn() const { return fcn_; }
    CheapTag const& ct() const { return ct_; }
    std::string const& process() const { return ct_.process(); }
    ProductID pid() const { return pid_; }
  private:
    BranchType bt_;
    std::string fcn_;
    CheapTag ct_;
    ProductID pid_;
  };

  void
  recreateLookups(ProductList const& prods,
                  BranchTypeLookup& pl,
                  BranchTypeLookup& el)
  {
    std::vector<PendingBTLEntry> pendingEntries;
    std::unordered_map<ProductID, CheapTag, ProductID::Hash> insertedABVs;
    for (auto const& val: prods) {
      auto const& procName = val.first.processName_;
      auto const pid = val.second.productID();
      auto const& prodFCN = val.first.friendlyClassName_;
      auto const bt = val.first.branchType_;
      pl[bt][prodFCN][procName].emplace_back(pid);
      // Look in the class of the product for a typedef named "value_type",
      // if there is one allow lookups by that type name too (and by all
      // of its base class names as well).
      art::TypeWithDict const TY {val.second.producedClassName()};
      if (TY.category() != art::TypeWithDict::Category::CLASSTYPE) {
        continue;
      }
      TClass* const TYc = TY.tClass();
      auto ET = mapped_type_of(TYc);
      if (ET || (ET = value_type_of(TYc))) {
        // The class of the product has a nested type, "mapped_type," or,
        // "value_type," so allow lookups by that type and all of its base
        // types too.
        auto const vtFCN = ET.friendlyClassName();
        el[bt][vtFCN][procName].emplace_back(pid);
        if (ET.category() == art::TypeWithDict::Category::CLASSTYPE) {
          // Repeat this for all public base classes of the value_type.
          std::vector<TClass*> bases;
          art::public_base_classes(ET.tClass(), bases);
          for (auto const BT: bases) {
            auto const btFCN = art::TypeID{BT->GetTypeInfo()}.friendlyClassName();
            el[bt][btFCN][procName].emplace_back(pid);
          }
        }
      }
      auto const& moduleLabel = val.first.moduleLabel_;
      auto const& instanceName = val.first.productInstanceName_;
      if (is_assns(TY.id())) {
        auto const TYName = TY.className();
        auto const baseName = name_of_assns_base(TYName);
        if (!baseName.empty()) {
          // We're an Assns<A, B, D>, with a base Assns<A, B>.
          TypeWithDict const base {baseName};
          // Add this to the list of "second-tier" products to register
          // later.
          assert(base.category() == art::TypeWithDict::Category::CLASSTYPE);
          auto const& baseFCN = base.friendlyClassName();
          pendingEntries.emplace_back(static_cast<art::BranchType>(bt),
                                      baseFCN,
                                      moduleLabel,
                                      instanceName,
                                      procName,
                                      pid);
        }
        else {
          // Add our pid to the list of real Assns<A, B, void>
          // products already registered.
          insertedABVs.emplace(pid, CheapTag{moduleLabel, instanceName, procName});
        }
      }
    }
    auto const iend = insertedABVs.cend();
    // Preserve useful ordering, only inserting if we don't already have
    // a *real* Assns<A, B, void> for that module label / instance name
    // combination.
    std::for_each(pendingEntries.cbegin(),
                  pendingEntries.cend(),
                  [&pl, &insertedABVs, iend](auto const& pe)
                  {
                    auto& pids = pl[pe.bt()][pe.fcn()][pe.process()];
                    if (pids.empty() ||
                        !std::any_of(pids.cbegin(), pids.cend(),
                                     [&insertedABVs, &iend, &pe](ProductID const pid) {
                                       auto i = insertedABVs.find(pid);
                                       return i != iend && pe.ct() == i->second;
                                     }))
                    {
                      pids.emplace_back(pe.pid());
                    }
                  });
  }
}

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

  checkDicts_(*bdp);
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
  perFileProds_[0].insert(productListEntry);
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

  productLookup_.assign(1u,{});
  elementLookup_.assign(1u,{});
  recreateLookups(productList_, productLookup_[0], elementLookup_[0]);
}

void
art::MasterProductRegistry::updateFromPrimaryFile(ProductList const& pl,
                                                  PerBranchTypePresence const& presList,
                                                  FileBlock const& fb)
{
  CET_ASSERT_ONLY_ONE_THREAD();

  perFileProds_.assign(1, {});
  perFilePresenceLookups_.assign(1u, {}); // Seed with one empty vector.
  productLookup_.assign(1u,{});
  elementLookup_.assign(1u,{});

  setPresenceLookups_(pl, presList);
  updateProductLists_(pl);

  recreateLookups(productList_, productLookup_[0], elementLookup_[0]);

  cet::for_all(productListUpdatedCallbacks_, [&fb](auto const& callback){ callback(fb); });
}

void
art::MasterProductRegistry::updateFromSecondaryFile(ProductList const& pl,
                                                    PerBranchTypePresence const& presList,
                                                    FileBlock const& fb)
{
  CET_ASSERT_ONLY_ONE_THREAD();

  // FIXME! Suppose a user has the following input-file dependency
  // tree:
  //
  //   PrimaryFile --> SecondaryFile1 -> TertiaryFile
  //               `-> SecondaryFile2
  //
  // Whenever each input file is opened, the product lookup tables are
  // increased by 1 element.  However, the lookups are flat.  This is
  // okay as long at the correct index is provided by the principals
  // whenever product lookups are being done.
  //
  // Unfortunately, this is not the case--each of the principals has
  // its own nextSecondaryFileIdx_ data member, which starts at 0 and
  // is the index provided to the MasterProductRegistry to retrieve
  // the "correct" lookup tables.
  //
  // To illustrate, if a product-lookup has failed using PrimaryFile,
  // then SecondaryFile1 is opened.  The relevant Principal (created
  // from PrimaryFile) has a nextSecondaryFileIdx_ member that is
  // incremented from 0 to 1, and MasterProductRegistry's product
  // lookups corresponding to index 1 are used for attempting to
  // retrieve the product.
  //
  // Now suppose SecondaryFile1 does not contain the product in
  // question, so TertiaryFile is opened.  The MasterProductRegistry's
  // lookups are expanded to accommodate TertiaryFile, and so the
  // index corresponding to TertiaryFile is 2 (according to the
  // MasterProductRegistry).  However, the "secondary" Principal
  // (created from SecondaryFile1) has a nextSecondaryFileIdx_ member
  // that is increased from 0 to 1, which represents TertiaryFile.  So
  // the index according to SecondaryFile1 for TertiaryFile is 1,
  // whereas it should be 2 for the MasterProductRegistry.  So the
  // lookups retrieved for TertiaryFile will be those of
  // SecondaryFile1.
  //
  // Note there are two cases when this is *not* a problem:
  //
  // (1) If the process history of TertiaryFile is an ancestor of
  //     SecondaryFile1, then the TertiaryFile's process history is a
  //     subset of SecondaryFile1's, and the correct groups will be
  //     queried (including those in SecondaryFile1 that are not part
  //     of TertiaryFile).
  //
  // (2) If there are only secondary input files (i.e. just
  //     SecondaryFile1 and SecondaryFile2 and no TertiaryFile), then
  //     the indexing of the principals and that of the
  //     MasterProductRegistry match.
  //
  // Ideally, this would be fixed by placing the lookups in the files
  // themselves, thus avoiding the awkward caching with a global
  // registry.

  perFileProds_.resize(perFileProds_.size()+1);
  perFilePresenceLookups_.resize(perFilePresenceLookups_.size()+1);
  productLookup_.resize(productLookup_.size()+1);
  elementLookup_.resize(elementLookup_.size()+1);

  setPresenceLookups_(pl, presList);
  updateProductLists_(pl);

  recreateLookups(perFileProds_.back(), productLookup_.back(), elementLookup_.back());

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
art::MasterProductRegistry::setPresenceLookups_(ProductList const& pl,
                                                PerBranchTypePresence const& presList)
{
  for (auto const& p : pl) {
    auto const& pd = p.second;
    auto const& presListForBT = presList[pd.branchType()];
    auto const pid = pd.productID();
    if (presListForBT.find(pid) != presListForBT.cend()) {
      perFilePresenceLookups_.back()[pd.branchType()].emplace(pid);
    }
  }
}

void
art::MasterProductRegistry::updateProductLists_(ProductList const& pl)
{
  for (auto const& val: pl) {
    auto const& pd = val.second;
    assert(!pd.produced());
    checkDicts_(pd);
    auto bk = BranchKey{pd};
    auto I = productList_.find(bk);
    if (I == productList_.end()) {
      // New product.
      productList_.emplace(bk, pd);
      perFileProds_.back().emplace(bk, pd);
      continue;
    }

    assert(combinable(I->second, pd));
    I->second.merge(pd);
    auto J = perFileProds_.back().find(bk);
    if (J == perFileProds_.back().end()) {
      // New product.
      perFileProds_.back().emplace(bk, pd);
      continue;
    }
    // Already had this product, combine in the additional parameter
    // sets and process descriptions.
    assert(combinable(J->second, pd));
    J->second.merge(pd);
  }
}

void
art::MasterProductRegistry::checkDicts_(BranchDescription const& productDesc)
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
art::operator<<(std::ostream& os, MasterProductRegistry const& mpr)
{
  mpr.print(os);
  return os;
}
