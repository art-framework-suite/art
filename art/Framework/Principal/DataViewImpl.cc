#include "art/Framework/Principal/DataViewImpl.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Principal/get_ProductDescription.h"
#include "art/Persistency/Provenance/ProcessHistoryRegistry.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/ProductSemantics.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/HorizontalRule.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetID.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace cet;
using namespace std;

namespace art {

  class EDProductGetter;

  DataViewImpl::
  ~DataViewImpl()
  {
  }

  DataViewImpl::
  DataViewImpl(BranchType const bt, Principal& principal, ModuleDescription const& md, bool const recordParents,
               RangeSet const& rs /* = RangeSet::invalid() */)
    : branchType_{bt}
    , principal_{principal}
    , md_{md}
    , recordParents_{recordParents}
    , gotProductIDs_{}
    , putProducts_{}
    , rangeSet_{rs}
  {
  }

  RunID
  DataViewImpl::
  runID() const
  {
    return principal_.runID();
  }

  SubRunID
  DataViewImpl::
  subRunID() const
  {
    return principal_.subRunID();
  }

  EventID
  DataViewImpl::
  eventID() const
  {
    return principal_.eventID();
  }

  RunNumber_t
  DataViewImpl::
  run() const
  {
    return principal_.run();
  }

  SubRunNumber_t
  DataViewImpl::
  subRun() const
  {
    return principal_.subRun();
  }

  EventNumber_t
  DataViewImpl::
  event() const
  {
    return principal_.event();
  }

  Timestamp const&
  DataViewImpl::
  beginTime() const
  {
    return principal_.beginTime();
  }

  Timestamp const&
  DataViewImpl::
  endTime() const
  {
    return principal_.endTime();
  }

  Timestamp
  DataViewImpl::
  time() const
  {
    return principal_.time();
  }

  bool
  DataViewImpl::
  isRealData() const
  {
    return principal_.isReal();
  }

  EventAuxiliary::ExperimentType
  DataViewImpl::
  experimentType() const
  {
    return principal_.ExperimentType();
  }

  History const&
  DataViewImpl::
  history() const
  {
    return principal_.history();
  }

  ProcessHistoryID const&
  DataViewImpl::
  processHistoryID() const
  {
    return principal_.history().processHistoryID();
  }

  size_t
  DataViewImpl::
  size() const
  {
    return putProducts_.size() + principal_.size();
  }

  EDProductGetter const*
  DataViewImpl::
  productGetter(ProductID const pid) const
  {
    return principal_.productGetter(pid);
  }

  bool
  DataViewImpl::
  getProcessParameterSet(string const& processName, fhicl::ParameterSet& ps) const
  {
    if (branchType_ != InEvent) {
      return false;
    }
    // Get the ProcessHistory for this event.
    ProcessHistory ph;
    if (!ProcessHistoryRegistry::get(principal_.history().processHistoryID(), ph)) {
      throw Exception(errors::NotFound)
        << "ProcessHistoryID "
        << principal_.history().processHistoryID()
        << " is not found in the ProcessHistoryRegistry.\n"
        << "This file is malformed.\n";
    }
    ProcessConfiguration config;
    bool const process_found = ph.getConfigurationForProcess(processName, config);
    if (process_found) {
      fhicl::ParameterSetRegistry::get(config.parameterSetID(), ps);
    }
    return process_found;
  }

  GroupQueryResult
  DataViewImpl::
  get_(TypeID const& tid, SelectorBase const& sel) const
  {
    return principal_.getBySelector(tid, sel);
  }

  GroupQueryResult
  DataViewImpl::
  getByProductID_(ProductID const pid) const
  {
    return principal_.getByProductID(pid);
  }

  void
  DataViewImpl::
  getMany_(TypeID const& tid, SelectorBase const& sel, std::vector<GroupQueryResult>& results) const
  {
    principal_.getMany(tid, sel, results);
  }

  GroupQueryResult
  DataViewImpl::
  getByLabel_(TypeID const& tid, string const& label, string const& instance, string const& process) const
  {
    return principal_.getByLabel(tid, label, instance, process);
  }

  int
  DataViewImpl::
  getMatchingSequenceByLabel_(TypeID const& elem, string const& label, string const& instance, std::vector<GroupQueryResult>& res) const
  {
    Selector sel(ModuleLabelSelector{label} && ProductInstanceNameSelector{instance});
    return principal_.getMatchingSequence(elem, sel, res);
  }

  int
  DataViewImpl::
  getMatchingSequenceByLabel_(TypeID const& elem, string const& label, string const& instance, string const& process,
                              std::vector<GroupQueryResult>& res) const
  {
    Selector sel(ModuleLabelSelector{label} && ProductInstanceNameSelector{instance} && ProcessNameSelector{process});
    return principal_.getMatchingSequence(elem, sel, res);
  }

  ProcessHistory const&
  DataViewImpl::
  processHistory() const
  {
    return principal_.processHistory();
  }

  void
  DataViewImpl::
  addToGotProductIDs(Provenance const& prov) const
  {
    if (!prov.productDescription().transient()) {
      gotProductIDs_.insert(prov.productID());
    }
    else {
      // If the product retrieved is transient, don't use its
      // ProductID; use the ProductID's of its parents.
      auto const& pids = prov.parents();
      gotProductIDs_.insert(pids.begin(), pids.end());
    }
  }

  void
  DataViewImpl::
  checkPutProducts(set<TypeLabel> const& expectedProducts)
  {
    vector<string> missing;
    for (auto const& typeLabel : expectedProducts) {
      if (putProducts_.find(typeLabel) != putProducts_.cend()) {
        continue;
      }
      ostringstream desc;
      desc << getBranchDescription(typeLabel.typeID(), typeLabel.productInstanceName());
      missing.emplace_back(desc.str());
    }
    if (!missing.empty()) {
      ostringstream errmsg;
      cet::HorizontalRule rule{25};
      errmsg << "The following products have been declared with 'produces',\n"
             << "but they have not been placed onto the event:\n"
             << rule('=') << '\n';
      for (auto const& desc : missing) {
        errmsg << desc
               << rule('=') << '\n';
      }
      throw Exception{errors::LogicError, "DataViewImpl::checkPutProducts"} << errmsg.str();
    }
  }

  void
  DataViewImpl::
  commit(bool const checkProducts, set<TypeLabel> const* expectedProducts)
  {
    if (checkProducts) {
      checkPutProducts(*expectedProducts);
    }
    for (auto& type_label_and_pmvalue : putProducts_) {
      auto& pmvalue = type_label_and_pmvalue.second;
      unique_ptr<ProductProvenance const> pp;
      if (branchType_ == InEvent) {
        vector<ProductID> gotPIDs;
        if (!gotProductIDs_.empty()) {
          gotPIDs.reserve(gotProductIDs_.size());
          gotPIDs.assign(gotProductIDs_.begin(), gotProductIDs_.end());
        }
        pp = make_unique<ProductProvenance const>(pmvalue.bd.productID(), productstatus::present(), gotPIDs);
      }
      else {
        pp = make_unique<ProductProvenance const>(pmvalue.bd.productID(), productstatus::present());
      }
      if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
        principal_.put(pmvalue.bd, move(pp), move(pmvalue.prod), move(make_unique<RangeSet>(pmvalue.rs)));
      }
      else {
        principal_.put(pmvalue.bd, move(pp), move(pmvalue.prod), move(make_unique<RangeSet>()));
      }
    };
    putProducts_.clear();
  }

  void
  DataViewImpl::
  commit()
  {
    for (auto& type_label_and_pmvalue : putProducts_) {
      auto& pmvalue = type_label_and_pmvalue.second;
      unique_ptr<ProductProvenance const> pp;
      pp = make_unique<ProductProvenance const>(pmvalue.bd.productID(), productstatus::present());
      if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
        principal_.put(pmvalue.bd, move(pp), move(pmvalue.prod), move(make_unique<RangeSet>(pmvalue.rs)));
      }
      else {
        principal_.put(pmvalue.bd, move(pp), move(pmvalue.prod), move(make_unique<RangeSet>()));
      }
    };
    putProducts_.clear();
  }

  void
  DataViewImpl::
  ensure_unique_product(size_t const nFound, TypeID const& typeID, string const& moduleLabel,
                        string const& instance, string const& processName) const
  {
    if (nFound == 1) {
      return;
    }
    Exception e(errors::ProductNotFound);
    e << "getView: Found "
      << (nFound == 0 ? "no products" : "more than one product")
      << " matching all criteria\n"
      << "Looking for sequence of type: " << typeID << "\n"
      << "Looking for module label: " << moduleLabel << "\n"
      << "Looking for productInstanceName: " << instance << "\n";
    if (!processName.empty()) {
      e << "Looking for processName: " << processName << "\n";
    }
    throw e;
  }

  BranchDescription const&
  DataViewImpl::
  getBranchDescription(TypeID const& type, string const& instance) const
  {
    return get_ProductDescription(type, md_.processName(), ProductMetaData::instance().productList(), branchType_, md_.moduleLabel(),
                                  instance);
  }

  void
  DataViewImpl::
  removeCachedProduct_(ProductID const pid) const
  {
    principal_.removeCachedProduct(pid);
  }

} // namespace art
