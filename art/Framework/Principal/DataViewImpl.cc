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
#include "cetlib/HorizontalRule.h"
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
  DataViewImpl(BranchType const bt,
               Principal const& principal,
               ModuleDescription const& md,
               bool const recordParents,
               RangeSet const& rs /* = RangeSet::invalid() */)
    : branchType_{bt}
    , principal_{principal}
    , md_{md}
    , recordParents_{recordParents}
    , retrievedProducts_{}
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

  // FIXME: This accessor is suspicious, and should be considered for
  // removable or updating.
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
  DataViewImpl::get_(WrappedTypeID const& wrapped, SelectorBase const& sel) const
  {
    return principal_.getBySelector(wrapped, sel);
  }

  GroupQueryResult
  DataViewImpl::
  getByProductID_(ProductID const pid) const
  {
    return principal_.getByProductID(pid);
  }

  DataViewImpl::GroupQueryResultVec
  DataViewImpl::getMany_(WrappedTypeID const& wrapped,
                         SelectorBase const& sel) const
  {
    return principal_.getMany(wrapped, sel);
  }

  GroupQueryResult
  DataViewImpl::getByLabel_(WrappedTypeID const& wrapped,
                            string const& label,
                            string const& productInstanceName,
                            string const& processName) const
  {
    return principal_.getByLabel(wrapped, label, productInstanceName, processName);
  }

  DataViewImpl::GroupQueryResultVec
  DataViewImpl::getMatchingSequenceByLabel_(string const& label,
                                            string const& productInstanceName,
                                            string const& processName) const
  {
    Selector const sel{ModuleLabelSelector{label} &&
                       ProductInstanceNameSelector{productInstanceName} &&
                       ProcessNameSelector{processName}};
    return principal_.getMatchingSequence(sel);
  }

  ProcessHistory const&
  DataViewImpl::
  processHistory() const
  {
    return principal_.processHistory();
  }

  void
  DataViewImpl::
  recordAsParent(Provenance const& prov) const
  {
    if (prov.productDescription().transient()) {
      // If the product retrieved is transient, don't use its
      // ProductID; use the ProductID's of its parents.
      auto const& parents = prov.parents();
      retrievedProducts_.insert(cbegin(parents), cend(parents));
    }
    else {
      retrievedProducts_.insert(prov.productID());
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
  commit(Principal& principal, bool const checkProducts, set<TypeLabel> const* expectedProducts)
  {
    if (checkProducts) {
      checkPutProducts(*expectedProducts);
    }
    for (auto& type_label_and_pmvalue : putProducts_) {
      auto& pmvalue = type_label_and_pmvalue.second;
      unique_ptr<ProductProvenance const> pp;
      if (branchType_ == InEvent) {
        vector<ProductID> gotPIDs;
        if (!retrievedProducts_.empty()) {
          gotPIDs.reserve(retrievedProducts_.size());
          gotPIDs.assign(retrievedProducts_.begin(), retrievedProducts_.end());
        }
        pp = make_unique<ProductProvenance const>(pmvalue.bd.productID(), productstatus::present(), gotPIDs);
      }
      else {
        pp = make_unique<ProductProvenance const>(pmvalue.bd.productID(), productstatus::present());
      }
      if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
        principal.put(pmvalue.bd, move(pp), move(pmvalue.prod), make_unique<RangeSet>(pmvalue.rs));
      }
      else {
        principal.put(pmvalue.bd, move(pp), move(pmvalue.prod), make_unique<RangeSet>());
      }
    };
    putProducts_.clear();
  }

  void
  DataViewImpl::removeNonViewableMatches_(TypeID const& requestedElementType,
                                          GroupQueryResultVec& results) const
  {
    // To determine if the requested view is allowed, the matched
    // 'results' (products) must be read.
    auto not_convertible = [&requestedElementType](auto const& query_result) {
      // Assns collections do not support views; we therefore do not
      // need to worry about an exception throw when calling
      // uniqueProduct.
      auto group = query_result.result();
      assert(group->productDescription().supportsView());
      auto p = group->uniqueProduct();
      return !detail::upcastAllowed(*p->typeInfo(), requestedElementType.typeInfo());
    };
    results.erase(std::remove_if(begin(results), end(results), not_convertible),
                  end(results));
  }

  void
  DataViewImpl::commit(Principal& principal)
  {
    for (auto& type_label_and_pmvalue : putProducts_) {
      auto& pmvalue = type_label_and_pmvalue.second;
      unique_ptr<ProductProvenance const> pp;
      pp = make_unique<ProductProvenance const>(pmvalue.bd.productID(), productstatus::present());
      if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
        principal.put(pmvalue.bd, move(pp), move(pmvalue.prod), make_unique<RangeSet>(pmvalue.rs));
      }
      else {
        principal.put(pmvalue.bd, move(pp), move(pmvalue.prod), make_unique<RangeSet>());
      }
    };
    putProducts_.clear();
  }

  void
  DataViewImpl::ensureUniqueProduct_(std::size_t const  nFound,
                                     TypeID      const& typeID,
                                     std::string const& moduleLabel,
                                     std::string const& productInstanceName,
                                     std::string const& processName) const
  {
    if (nFound == 1) {
      return;
    }
    Exception e{errors::ProductNotFound};
    e << "getView: Found "
      << (nFound == 0 ? "no products" : "more than one product")
      << " matching all criteria\n"
      << "Looking for sequence of type: " << typeID << "\n"
      << "Looking for module label: " << moduleLabel << "\n"
      << "Looking for productInstanceName: " << productInstanceName << "\n";
    if (!processName.empty()) {
      e << "Looking for processName: " << processName << "\n";
    }
    throw e;
  }

  BranchDescription const&
  DataViewImpl::
  getBranchDescription(TypeID const& type, string const& instance) const
  {
    return get_ProductDescription(type,
                                  md_.processName(),
                                  ProductMetaData::instance().productDescriptions(branchType_),
                                  branchType_,
                                  md_.moduleLabel(),
                                  instance);
  }

  void
  DataViewImpl::
  removeCachedProduct_(ProductID const pid) const
  {
    principal_.removeCachedProduct(pid);
  }

} // namespace art
