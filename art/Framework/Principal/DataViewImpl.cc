#include "art/Framework/Principal/DataViewImpl.h"

#include "art/Framework/Principal/Consumer.h"
#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/get_ProductDescription.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProductStatus.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>

using namespace cet;
using namespace std;

namespace art {

  DataViewImpl::DataViewImpl(Principal const& pcpl,
                             ModuleDescription const& md,
                             BranchType const branchType,
                             bool const recordParents,
                             cet::exempt_ptr<Consumer> consumer) :
    principal_{pcpl},
    md_{md},
    branchType_{branchType},
    recordParents_{recordParents},
    consumer_{consumer}
  {}

  size_t
  DataViewImpl::size() const
  {
    return putProducts_.size() + principal_.size();
  }

  GroupQueryResult
  DataViewImpl::get_(WrappedTypeID const& wrapped, SelectorBase const& sel) const
  {
    return principal_.getBySelector(wrapped, sel);
  }

  GroupQueryResult
  DataViewImpl::getByProductID_(ProductID const pid) const
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
      retrievedProducts_.insert(cbegin(parents), cend(parents));
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
    result.assign(cbegin(retrievedProducts_), cend(retrievedProducts_));
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
  DataViewImpl::ensureUniqueProduct_(std::size_t const  nFound,
                                     TypeID      const& typeID,
                                     std::string const& moduleLabel,
                                     std::string const& productInstanceName,
                                     std::string const& processName) const
  {
    if (nFound == 1) return;

    Exception e{errors::ProductNotFound};
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
