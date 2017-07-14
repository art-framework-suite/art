#include "art/Framework/Principal/DataViewImpl.h"

#include "art/Framework/Principal/ConsumesRecorder.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/get_ProductDescription.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "cetlib/HorizontalRule.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>

using namespace cet;
using namespace std;

namespace art {

  DataViewImpl::DataViewImpl(Principal const& pcpl,
                             ModuleDescription const& md,
                             BranchType const branchType,
                             bool const recordParents,
                             ConsumesRecorder& consumesRecorder) :
    principal_{pcpl},
    md_{md},
    branchType_{branchType},
    recordParents_{recordParents},
    consumesRecorder_{consumesRecorder}
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

  GroupQueryResult
  DataViewImpl::getByProductID_(ProductID const pid) const
  {
    return principal_.getByProductID(pid);
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
  DataViewImpl::recordAsParent(Provenance const& prov) const
  {
    if (prov.productDescription().transient()) {
      // If the product retrieved is transient, don't use its
      // ProductID; use the ProductID's of its parents.
      auto const& parents = prov.parents();
      retrievedProducts_.insert(std::cbegin(parents), std::cend(parents));
    }
    else {
      retrievedProducts_.insert(prov.productID());
    }
  }

  DataViewImpl::RetrievedProductIDs
  DataViewImpl::retrievedProductIDs() const
  {
    std::vector<ProductID> result;
    result.reserve(retrievedProducts_.size());
    result.assign(std::cbegin(retrievedProducts_), std::cend(retrievedProducts_));
    return result;
  }

  void
  DataViewImpl::checkPutProducts(bool const checkProducts,
                                 std::set<TypeLabel> const& expectedProducts,
                                 TypeLabelMap const& putProducts)
  {
    if (!checkProducts) return;

    std::vector<std::string> missing;
    for (auto const& typeLabel : expectedProducts) {
      if (putProducts.find(typeLabel) != putProducts.cend()) continue;

      std::ostringstream desc;
      desc << getProductDescription(typeLabel.typeID(), typeLabel.productInstanceName());
      missing.emplace_back(desc.str());
    }

    if (!missing.empty()) {
      std::ostringstream errmsg;
      HorizontalRule rule{25};
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

  BranchDescription const&
  DataViewImpl::getProductDescription(TypeID const& type,
                                      string const& productInstanceName) const
  {
    return get_ProductDescription(type,
                                  md_.processName(),
                                  ProductMetaData::instance().productList(),
                                  branchType_,
                                  md_.moduleLabel(),
                                  productInstanceName);
  }

  void
  DataViewImpl::ensureUniqueProduct_(std::size_t const  nFound,
                                     TypeID      const& typeID,
                                     std::string const& moduleLabel,
                                     std::string const& productInstanceName,
                                     std::string const& processName) const
  {
    if (nFound == 1) return;

    art::Exception e(art::errors::ProductNotFound);
    e << "getView: Found "
      << (nFound == 0 ? "no products"
          : "more than one product"
          )
      << " matching all criteria\n"
      << "Looking for sequence of type: " << typeID << "\n"
      << "Looking for module label: " << moduleLabel << "\n"
      << "Looking for productInstanceName: " << productInstanceName << "\n";
    if (!processName.empty())
      e << "Looking for processName: "<< processName <<"\n";
    throw e;
  }

  void
  DataViewImpl::removeCachedProduct_(ProductID const pid) const
  {
    principal_.removeCachedProduct(pid);
  }

}  // art
