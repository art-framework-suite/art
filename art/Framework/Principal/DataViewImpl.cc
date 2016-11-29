#include "art/Framework/Principal/DataViewImpl.h"

#include "art/Framework/Principal/Principal.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Principal/Selector.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>

using namespace cet;
using namespace std;

namespace art {

  DataViewImpl::DataViewImpl(Principal & pcpl,
                             ModuleDescription const& md,
                             BranchType const branchType)  :
    putProducts_(),
    principal_(pcpl),
    md_(md),
    branchType_(branchType)
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
                                     bool stopIfProcessHasMatch) const
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
                                            bool stopIfProcessHasMatch) const
  {
    art::Selector sel(art::ModuleLabelSelector(label) &&
                      art::ProductInstanceNameSelector(productInstanceName));

    int n = principal_.getMatchingSequence(elementType,
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
                                            bool stopIfProcessHasMatch) const
  {
    art::Selector sel(art::ModuleLabelSelector(label) &&
                      art::ProductInstanceNameSelector(productInstanceName) &&
                      art::ProcessNameSelector(processName) );

    int n = principal_.getMatchingSequence(elementType,
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
      for ( auto const& bd : missing ) {
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
    string const friendlyClassName {type.friendlyClassName()};
    BranchKey bk {friendlyClassName,
                  md_.moduleLabel(),
                  productInstanceName,
                  md_.processName(),
                  branchType_};
    auto const& pl = ProductMetaData::instance().productList();
    auto it = pl.find(bk);

    if (it == pl.end()) {
      throw art::Exception{art::errors::ProductRegistrationFailure, "DataViewImpl::getBranchDescription"}
        << "Illegal attempt to retrieve an unregistered product.\n"
        << "No product is registered for\n"
        << "  process name:                '" << bk.processName_ << "'\n"
        << "  module label:                '" << bk.moduleLabel_ << "'\n"
        << "  product friendly class name: '" << bk.friendlyClassName_ << "'\n"
        << "  product instance name:       '" << bk.productInstanceName_ << "'\n"
        << "  branch type:                 '" << branchType_ << "'\n"
        << "Registered products currently:\n"
        << ProductMetaData::instance()
        << '\n';
    }
    return it->second;
  }

  void
  DataViewImpl::removeCachedProduct_(BranchID const bid) const
  {
    principal().removeCachedProduct(bid);
  }

}  // art
