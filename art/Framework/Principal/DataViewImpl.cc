#include "art/Framework/Principal/DataViewImpl.h"

#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/get_BranchDescription.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>

using namespace cet;
using namespace std;

namespace art {

  DataViewImpl::DataViewImpl(Principal const& pcpl,
                             ModuleDescription const& md,
                             BranchType const branchType)  :
    principal_{pcpl},
    md_{md},
    branchType_{branchType}
  {}

  size_t
  DataViewImpl::size() const
  {
    return putProducts_.size() + principal_.size();
  }

  GroupQueryResult
  DataViewImpl::get_(TypeID const& tid, SelectorBase const& sel) const
  {
    return principal_.getBySelector(tid, sel);
  }

  void
  DataViewImpl::getMany_(TypeID const& tid,
                         SelectorBase const& sel,
                         GroupQueryResultVec& results) const
  {
    principal_.getMany(tid, sel, results);
  }

  GroupQueryResult
  DataViewImpl::getByLabel_(TypeID const& tid,
                            string const& label,
                            string const& productInstanceName,
                            string const& processName) const
  {
    return principal_.getByLabel(tid, label, productInstanceName, processName);
  }

  void
  DataViewImpl::getManyByType_(TypeID const& tid,
                               GroupQueryResultVec& results) const
  {
    principal_.getManyByType(tid, results);
  }

  int
  DataViewImpl::getMatchingSequence_(TypeID const& elementType,
                                     SelectorBase const& selector,
                                     GroupQueryResultVec& results,
                                     bool const stopIfProcessHasMatch) const
  {
    return principal_.getMatchingSequence(elementType,
                                          selector,
                                          results,
                                          stopIfProcessHasMatch);
  }

  int
  DataViewImpl::getMatchingSequenceByLabel_(TypeID const& elementType,
                                            string const& label,
                                            string const& productInstanceName,
                                            GroupQueryResultVec& results,
                                            bool const stopIfProcessHasMatch) const
  {
    art::Selector sel(art::ModuleLabelSelector{label} &&
                      art::ProductInstanceNameSelector{productInstanceName});

    int const n = principal_.getMatchingSequence(elementType,
                                                 sel,
                                                 results,
                                                 stopIfProcessHasMatch);
    return n;
  }

  int
  DataViewImpl::getMatchingSequenceByLabel_(TypeID const& elementType,
                                            string const& label,
                                            string const& productInstanceName,
                                            string const& processName,
                                            GroupQueryResultVec& results,
                                            bool const stopIfProcessHasMatch) const
  {
    art::Selector sel(art::ModuleLabelSelector{label} &&
                      art::ProductInstanceNameSelector{productInstanceName} &&
                      art::ProcessNameSelector{processName});

    int const n = principal_.getMatchingSequence(elementType,
                                                 sel,
                                                 results,
                                                 stopIfProcessHasMatch);
    return n;
  }

  ProcessHistory const&
  DataViewImpl::processHistory() const
  {
    return principal_.processHistory();
  }

  void
  DataViewImpl::checkPutProducts(bool const checkProducts,
                                 ProducedMap const& expectedBids,
                                 BranchIDsMap const& products)
  {
    if (!checkProducts) return;

    std::vector<std::string> missing;
    for (auto const& bid : expectedBids) {
      auto const& putBids = products;
      if (putBids.find(bid.first) == putBids.cend())
        missing.emplace_back(bid.second);
    }

    if (!missing.empty()) {
      std::ostringstream errmsg;
      errmsg << "The following products have been declared with 'produces',\n"
             << "but they have not been placed onto the event:\n"
             << "=========================\n";
      for (auto const& bd : missing) {
        errmsg << bd
               << "=========================\n";
      }
      throw Exception{errors::LogicError, "DataViewImpl::checkPutProducts"} << errmsg.str();
    }
  }

  BranchDescription const&
  DataViewImpl::getBranchDescription(TypeID const& type,
                                     string const& productInstanceName) const
  {
    return get_BranchDescription(type,
                                 md_.processName(),
                                 ProductMetaData::instance().productList(),
                                 branchType_,
                                 md_.moduleLabel(),
                                 productInstanceName);
  }

  void
  DataViewImpl::removeCachedProduct_(BranchID const bid) const
  {
    principal_.removeCachedProduct(bid);
  }

}  // art
