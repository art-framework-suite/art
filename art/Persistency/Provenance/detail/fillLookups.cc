#include "art/Persistency/Provenance/detail/fillLookups.h"
// vim: set sw=2:

#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Persistency/Provenance/TypeWithDict.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/FriendlyName.h"
#include "canvas/Utilities/WrappedClassName.h"

#include <cassert>
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

}


std::pair<art::ProductLookup_t, art::ViewLookup_t>
art::detail::fillLookups(ProductList const& prods)
{
  // Computing the product lookups does not rely on any ROOT facilities.
  ProductLookup_t pl;
  std::vector<PendingBTLEntry> pendingEntries;
  std::unordered_map<ProductID, CheapTag, ProductID::Hash> insertedABVs;
  for (auto const& val: prods) {
    auto const& procName = val.first.processName_;
    auto const pid = val.second.productID();
    auto const& prodFCN = val.first.friendlyClassName_;
    auto const bt = val.first.branchType_;
    pl[bt][prodFCN][procName].emplace_back(pid);

    // Additional work only for Assns lookup
    auto const& moduleLabel = val.first.moduleLabel_;
    auto const& instanceName = val.first.productInstanceName_;
    auto const& className = val.second.producedClassName();

    if (!is_assns(className)) continue;

    auto const baseName = name_of_assns_base(className);
    if (!baseName.empty()) {
      // We're an Assns<A, B, D>, with a base Assns<A, B>.
      pendingEntries.emplace_back(static_cast<art::BranchType>(bt),
                                  art::friendlyname::friendlyName(baseName),
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

  // Form element lookups for views.  Right now we rely on ROOT to
  // tell us the list of allowed base classes.
  ViewLookup_t vl;
  for (auto const& val: prods) {
    auto const& procName = val.first.processName_;
    auto const pid = val.second.productID();
    auto const bt = val.first.branchType_;

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
      vl[bt][vtFCN][procName].emplace_back(pid);
      if (ET.category() == art::TypeWithDict::Category::CLASSTYPE) {
        // Repeat this for all public base classes of the value_type.
        std::vector<TClass*> bases;
        art::public_base_classes(ET.tClass(), bases);
        for (auto const BT: bases) {
          auto const btFCN = art::TypeID{BT->GetTypeInfo()}.friendlyClassName();
          vl[bt][btFCN][procName].emplace_back(pid);
        }
      }
    }
  }
  return std::make_pair(pl, vl);
}
