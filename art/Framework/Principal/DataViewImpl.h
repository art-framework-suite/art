#ifndef art_Framework_Principal_DataViewImpl_h
#define art_Framework_Principal_DataViewImpl_h

// ======================================================================
//
// DataViewImpl - This is the implementation for accessing EDProducts
// and inserting new EDproducts.
//
// Getting Data
//
// The art::DataViewImpl class provides many 'get*" methods for
// getting data it contains.
//
// The primary method for getting data is to use getByLabel(). The
// labels are the label of the module assigned in the configuration
// file and the 'product instance label' (which can be omitted in the
// case the 'product instance label' is the default value).  The C++
// type of the product plus the two labels uniquely identify a product
// in the DataViewImpl.
//
// We use an Event in the examples, but a Run or a SubRun can also
// hold products.
//
//   art::Handle<AppleCollection> apples;
//   event.getByLabel("tree", apples);
//
//   art::Handle<FruitCollection> fruits;
//   event.getByLabel("market", "apples", fruits);
//
// Putting Data
//
//   auto pApples = std::make_unique<AppleCollection>();
//   // fill the collection
//   ...
//   event.put(std::move(pApples));
//
//   auto pFruits = std::make_unique<FruitCollection>();
//   // fill the collection
//   ...
//   event.put(std::move(pFruits), "apples");
//
// ======================================================================

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Consumer.h"
#include "art/Framework/Principal/ProductInfo.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Common/fwd.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"

#include <ostream>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {
  template <typename PROD>
  std::ostream&
  operator<<(std::ostream& os, Handle<PROD> const& h);
}

class art::DataViewImpl {
public:

  DataViewImpl(DataViewImpl const&) = delete;
  DataViewImpl& operator=(DataViewImpl const&) = delete;

  explicit DataViewImpl(Principal const& p,
                        ModuleDescription const& md,
                        BranchType bt,
                        bool recordParents,
                        cet::exempt_ptr<Consumer> consumer);

  size_t size() const;

  template <typename PROD>
  bool
  get(SelectorBase const&, Handle<PROD>& result) const;

  template <typename PROD>
  bool
  get(ProductID const pid, Handle<PROD>& result) const;

  template <typename PROD>
  bool
  getByLabel(std::string const& label,
             std::string const& productInstanceName,
             Handle<PROD>& result) const;

  template <typename PROD>
  bool
  getByLabel(std::string const& label,
             std::string const& productInstanceName,
             std::string const& processName,
             Handle<PROD>& result) const;

  /// same as above, but using the InputTag class
  template <typename PROD>
  PROD const&
  getByLabel(InputTag const& tag) const;

  template <typename PROD>
  bool
  getByLabel(InputTag const& tag, Handle<PROD>& result) const;

  template <typename PROD>
  PROD const*
  getPointerByLabel(InputTag const& tag) const;

  template <typename PROD>
  bool
  getByToken(ProductToken<PROD> const& token, Handle<PROD>& result) const;

  template <typename PROD>
  ValidHandle<PROD>
  getValidHandle(InputTag const& tag) const;

  template <typename PROD>
  ValidHandle<PROD>
  getValidHandle(ProductToken<PROD> const& token) const;

  template <typename PROD>
  void
  getMany(SelectorBase const&, std::vector<Handle<PROD>>& results) const;

  template <typename PROD>
  void
  getManyByType(std::vector<Handle<PROD>>& results) const;

  // If getView returns true, then result.isValid() is certain to be
  // true -- but the View may still be empty.
  template <typename ELEMENT>
  std::size_t
  getView(std::string const& moduleLabel,
          std::string const& productInstanceName,
          std::vector<ELEMENT const*>& result) const;

  template <typename ELEMENT>
  std::size_t
  getView(InputTag const& tag,
          std::vector<ELEMENT const*>& result) const;

  template <typename ELEMENT>
  std::size_t
  getView(ViewToken<ELEMENT> const& token,
          std::vector<ELEMENT const*>& result) const;

  template <typename ELEMENT>
  bool
  getView(std::string const& moduleLabel,
          std::string const& instanceName,
          View<ELEMENT>& result) const;

  template <typename ELEMENT>
  bool
  getView(InputTag const& tag, View<ELEMENT>& result) const;

  template <typename ELEMENT>
  bool
  getView(ViewToken<ELEMENT> const& tag, View<ELEMENT>& result) const;

  template <typename PROD>
  bool
  removeCachedProduct(Handle<PROD>& h) const;

  ProcessHistory const&
  processHistory() const;

  struct PMValue {

    PMValue(std::unique_ptr<EDProduct>&& p,
            BranchDescription const& b,
            RangeSet const& r)
      : prod{std::move(p)}, pd{b}, rs{r}
    {}

    std::unique_ptr<EDProduct> prod;
    BranchDescription const& pd;
    RangeSet rs;
  };

  using RetrievedProductIDs = std::vector<ProductID>;
  using RetrievedProductSet = std::set<ProductID>;
  using TypeLabelMap = std::map<TypeLabel, PMValue>;

protected:

  void recordAsParent(Provenance const& prov) const;

  TypeLabelMap&       putProducts()       {return putProducts_;}
  TypeLabelMap const& putProducts() const {return putProducts_;}

  // Return the map of products that was retrieved via get*.  The
  // retrievedProducts_ member is used to form the sequence of
  // ProductIDs that serve as the "parents" to any put products.
  RetrievedProductSet const& retrievedProducts() const {return retrievedProducts_;}

  // Convert the retrievedProducts_ member to just the sequence of
  // ProductIDs corresponding to product parents.
  RetrievedProductIDs retrievedProductIDs() const;

  void
  checkPutProducts(bool checkProducts,
                   std::set<TypeLabel> const& expectedProducts,
                   TypeLabelMap const& putProducts);

  BranchDescription const&
  getProductDescription(TypeID const& type, std::string const& productInstanceName) const;

  using GroupQueryResultVec = std::vector<GroupQueryResult>;

private:

  void
  ensureUniqueProduct_(std::size_t nFound,
                       TypeID const& typeID,
                       std::string const& moduleLabel,
                       std::string const& productInstanceName,
                       std::string const& processName) const;

  // The following 'get' functions serve to isolate the DataViewImpl class
  // from the Principal class.
  GroupQueryResult
  get_(TypeID const& tid,
       TypeID const& wrapped_tid,
       SelectorBase const&) const;

  GroupQueryResult
  getByProductID_(ProductID const pid) const;

  GroupQueryResult
  getByLabel_(TypeID const& productType,
              TypeID const& wrappedProductType,
              std::string const& label,
              std::string const& productInstanceName,
              std::string const& processName) const;

  void
  getMany_(TypeID const& productType,
           TypeID const& wrappedProductType,
           SelectorBase const& sel,
           GroupQueryResultVec& results) const;

  void
  getManyByType_(TypeID const& productType,
                 TypeID const& wrappedProductType,
                 GroupQueryResultVec& results) const;

  GroupQueryResultVec
  getMatchingSequenceByLabel_(TypeID const& elementType,
                              std::string const& label,
                              std::string const& productInstanceName,
                              std::string const& processName) const;



  // If getView returns true, then result.isValid() is certain to be
  // true -- but the View may still be empty.
  template <typename ELEMENT>
  GroupQueryResultVec
  getView_(std::string const& moduleLabel,
           std::string const& productInstanceName,
           std::string const& processName) const;

  template <typename ELEMENT>
  void
  fillView_(GroupQueryResult& bh,
            std::vector<ELEMENT const*>& result) const;

  void removeCachedProduct_(ProductID const pid) const;

  //------------------------------------------------------------
  // Data members
  //

  // putProducts_ is the holding pen for EDProducts inserted into this
  // DataViewImpl. Pointers in these collections own the products to
  // which they point.
  TypeLabelMap putProducts_{};

  // gotProductIDs_ must be mutable because it records all 'gets',
  // which do not logically modify the DataViewImpl. gotProductIDs_ is
  // merely a cache reflecting what has been retrieved from the
  // Principal class.
  mutable RetrievedProductSet retrievedProducts_{};

  // Each DataViewImpl must have an associated Principal, used as the
  // source of all 'gets' and the target of 'puts'.
  Principal const& principal_;

  // Each DataViewImpl must have a description of the module executing
  // the "transaction" which the DataViewImpl represents.
  ModuleDescription const& md_;

  // Is this an Event, a SubRun, or a Run.
  BranchType const branchType_;

  // Should we record the parents for any products that will be put.
  bool const recordParents_;

  // The consumer is access to validate that the product being
  // retrieved has been declared in a user's module c'tor to be a
  // consumable product..
  cet::exempt_ptr<Consumer> consumer_;
};

template <typename PROD>
inline
std::ostream&
art::operator<<(std::ostream& os, Handle<PROD> const& h)
{
  os << h.product() << " " << h.provenance() << " " << h.id();
  return os;
}

// Implementation of DataViewImpl member templates. See
// DataViewImpl.cc for the implementation of non-template members.

template <typename PROD>
inline
bool
art::DataViewImpl::get(SelectorBase const& sel,
                       Handle<PROD>& result) const
{
  result.clear(); // Is this the correct thing to do if an exception is thrown?
  // We do *not* track whether consumes was called for a SelectorBase.
  GroupQueryResult bh = get_(TypeID{typeid(PROD)}, TypeID{typeid(Wrapper<PROD>)}, sel);
  convert_handle(bh, result);
  bool const ok{bh.succeeded() && !result.failedToGet()};
  if (recordParents_ && ok) {
    recordAsParent(*result.provenance());
  }
  return ok;
}

template <typename PROD>
bool
art::DataViewImpl::get(ProductID const pid, Handle<PROD>& result) const
{
  result.clear(); // Is this the correct thing to do if an exception is thrown?
  // We do *not* track whether consumes was called for a ProductID.
  GroupQueryResult bh = getByProductID_(pid);
  convert_handle(bh, result);
  bool const ok{bh.succeeded() && !result.failedToGet()};
  if (recordParents_ && ok) {
    recordAsParent(*result.provenance());
  }
  return ok;
}

template <typename PROD>
inline
bool
art::DataViewImpl::getByLabel(InputTag const& tag, Handle<PROD>& result) const
{
  return getByLabel<PROD>(tag.label(), tag.instance(), tag.process(), result);
}

template <typename PROD>
inline
bool
art::DataViewImpl::getByLabel(std::string const& label,
                              std::string const& productInstanceName,
                              Handle<PROD>& result) const
{
  return getByLabel<PROD>(label, productInstanceName, {}, result);
}

template <typename PROD>
inline
bool
art::DataViewImpl::getByLabel(std::string const& label,
                              std::string const& productInstanceName,
                              std::string const& processName,
                              Handle<PROD>& result) const
{
  result.clear(); // Is this the correct thing to do if an exception is thrown?
  TypeID const tid{typeid(PROD)};
  ProductInfo const pinfo{ProductInfo::ConsumableType::Product, tid, label, productInstanceName, processName};
  consumer_->validateConsumedProduct(branchType_, pinfo);
  GroupQueryResult bh = getByLabel_(tid, TypeID{typeid(Wrapper<PROD>)}, label, productInstanceName, processName);
  convert_handle(bh, result);
  bool const ok{bh.succeeded() && !result.failedToGet()};
  if (recordParents_ && ok) {
    recordAsParent(*result.provenance());
  }
  return ok;
}


template <typename PROD>
inline
PROD const&
art::DataViewImpl::getByLabel(InputTag const& tag) const
{
  Handle<PROD> h;
  getByLabel(tag, h);
  return *h;
}

template <typename PROD>
inline
bool
art::DataViewImpl::getByToken(ProductToken<PROD> const& token, Handle<PROD>& result) const
{
  auto const& tag = token.inputTag_;
  return getByLabel(tag.label(), tag.instance(), tag.process(), result);
}

template <typename PROD>
inline
PROD const*
art::DataViewImpl::getPointerByLabel(InputTag const& tag) const
{
  Handle<PROD> h;
  getByLabel(tag, h);
  return &(*h);
}

template <typename PROD>
inline
art::ValidHandle<PROD>
art::DataViewImpl::getValidHandle(InputTag const& tag) const
{
  Handle<PROD> h;
  getByLabel(tag, h);
  return ValidHandle<PROD>(&(*h), *h.provenance());
}

template <typename PROD>
inline
art::ValidHandle<PROD>
art::DataViewImpl::getValidHandle(ProductToken<PROD> const& token) const
{
  return getValidHandle<PROD>(token.inputTag_);
}

template <typename PROD>
inline
void
art::DataViewImpl::getMany(SelectorBase const& sel,
                           std::vector<Handle<PROD>>& results) const
{
  TypeID const tid{typeid(PROD)};
  consumer_->validateConsumedProduct(branchType_, ProductInfo{ProductInfo::ConsumableType::Many, tid});
  GroupQueryResultVec bhv;
  getMany_(tid, TypeID{typeid(Wrapper<PROD>)}, sel, bhv);

  std::vector<Handle<PROD>> products;
  for (auto const& qr : bhv) {
    Handle<PROD> result;
    convert_handle(qr, result);
    products.push_back(result);
  }
  results.swap(products);

  if (!recordParents_) return;

  for (auto const& h : results)
    recordAsParent(*h.provenance());
}

template <typename PROD>
inline
void
art::DataViewImpl::getManyByType(std::vector<Handle<PROD>>& results) const
{
  getMany(MatchAllSelector{}, results);
}

template <typename ELEMENT>
art::DataViewImpl::GroupQueryResultVec
art::DataViewImpl::getView_(std::string const& moduleLabel,
                            std::string const& productInstanceName,
                            std::string const& processName) const
{
  TypeID const typeID{typeid(ELEMENT)};
  ProductInfo const pinfo{ProductInfo::ConsumableType::ViewElement,
      typeID,
      moduleLabel,
      productInstanceName,
      processName};
  consumer_->validateConsumedProduct(branchType_, pinfo);
  auto bhv = getMatchingSequenceByLabel_(typeID,
                                         moduleLabel,
                                         productInstanceName,
                                         processName);
  ensureUniqueProduct_(bhv.size(), typeID,
                       moduleLabel, productInstanceName, processName);
  return bhv;
}  // getView_<>()

template <typename ELEMENT>
std::size_t
art::DataViewImpl::getView(std::string const& moduleLabel,
                           std::string const& productInstanceName,
                           std::vector<ELEMENT const*>& result) const
{
  auto bhv = getView_<ELEMENT>(moduleLabel, productInstanceName, {});
  std::size_t const orig_size = result.size();
  fillView_(bhv[0], result);
  return result.size() - orig_size;
}  // getView<>()

template <typename ELEMENT>
std::size_t
art::DataViewImpl::getView(InputTag const& tag,
                           std::vector<ELEMENT const*>& result) const
{
  auto bhv = getView_<ELEMENT>(tag.label(), tag.instance(), tag.process());
  std::size_t const orig_size = result.size();
  fillView_(bhv[0], result);
  return result.size() - orig_size;
}  // getView<>()

template <typename ELEMENT>
bool
art::DataViewImpl::getView(std::string const& moduleLabel,
                           std::string const& productInstanceName,
                           View<ELEMENT>& result) const
{
  auto bhv = getView_<ELEMENT>(moduleLabel, productInstanceName, {});
  fillView_(bhv[0], result.vals());
  result.set_innards(bhv[0].result()->productID(), bhv[0].result()->uniqueProduct());
  return true;
}

template <typename ELEMENT>
bool
art::DataViewImpl::getView(InputTag const& tag, View<ELEMENT>& result) const
{
  auto bhv = getView_<ELEMENT>(tag.label(), tag.instance(), tag.process());
  fillView_(bhv[0], result.vals());
  result.set_innards(bhv[0].result()->productID(), bhv[0].result()->uniqueProduct());
  return true;
}

template <typename ELEMENT>
bool
art::DataViewImpl::getView(ViewToken<ELEMENT> const& token, View<ELEMENT>& result) const
{
  return getView(token.inputTag_, result);
}

// ----------------------------------------------------------------------

template <typename ELEMENT>
void
art::DataViewImpl::fillView_(GroupQueryResult& bh,
                             std::vector<ELEMENT const*>& result) const
{
  std::vector<void const*> erased_ptrs;
  bh.result()->uniqueProduct()->fillView(erased_ptrs);
  recordAsParent(Provenance{bh.result()});

  std::vector<ELEMENT const*> vals;
  cet::transform_all(erased_ptrs,
                     std::back_inserter(vals),
                     [](auto p) {
                       return static_cast<ELEMENT const*>(p);
                     });
  result.swap(vals);
}

template <typename PROD>
bool
art::DataViewImpl::removeCachedProduct(Handle<PROD>& h) const
{
  bool result{false};
  if (h.isValid() && !h.provenance()->produced()) {
    removeCachedProduct_(h.id());
    h.clear();
    result = true;
  }
  return result;
}

#endif /* art_Framework_Principal_DataViewImpl_h */

// Local Variables:
// mode: c++
// End:
