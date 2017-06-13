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
                        bool recordParents);

  size_t size() const;

  template <typename PROD>
  bool
  get(SelectorBase const&, Handle<PROD>& result) const;

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
  ValidHandle<PROD>
  getValidHandle(InputTag const& tag) const;

  template <typename PROD>
  void
  getMany(SelectorBase const&, std::vector<Handle<PROD>>& results) const;

  template <typename PROD>
  void
  getManyByType(std::vector<Handle<PROD>>& results) const;

  template <typename PROD>
  bool removeCachedProduct(Handle<PROD>& h) const;

  ProcessHistory const&
  processHistory() const;

  struct PMValue {

    PMValue(std::unique_ptr<EDProduct>&& p,
            BranchDescription const& b,
            RangeSet const& r)
      : prod{std::move(p)}, bd{b}, rs{r}
    {}

    std::unique_ptr<EDProduct> prod;
    BranchDescription const& bd;
    RangeSet rs;
  };

  using RetrievedProductSet = std::set<BranchID>;
  using TypeLabelMap = std::map<TypeLabel, PMValue>;

protected:

  void addToGotBranchIDs(Provenance const& prov) const;

  TypeLabelMap      & putProducts()       {return putProducts_;}
  TypeLabelMap const& putProducts() const {return putProducts_;}

  RetrievedProductSet& retrievedProducts() {return gotBranchIDs_;}

  void
  checkPutProducts(bool checkProducts,
                   std::set<TypeLabel> const& expectedProducts,
                   TypeLabelMap const& putProducts);

  BranchDescription const&
  getBranchDescription(TypeID const& type, std::string const& productInstanceName) const;

  using GroupQueryResultVec = std::vector<GroupQueryResult>;

  // The following 'get' functions serve to isolate the DataViewImpl class
  // from the Principal class.
  int
  getMatchingSequenceByLabel_(TypeID const& elementType,
                              std::string const& label,
                              std::string const& productInstanceName,
                              GroupQueryResultVec& results,
                              bool stopIfProcessHasMatch) const;

  int
  getMatchingSequenceByLabel_(TypeID const& elementType,
                              std::string const& label,
                              std::string const& productInstanceName,
                              std::string const& processName,
                              GroupQueryResultVec& results,
                              bool stopIfProcessHasMatch) const;

private:

  GroupQueryResult
  get_(TypeID const& tid, SelectorBase const&) const;

  GroupQueryResult
  getByLabel_(TypeID const& tid,
              std::string const& label,
              std::string const& productInstanceName,
              std::string const& processName) const;

  void
  getMany_(TypeID const& tid,
           SelectorBase const& sel,
           GroupQueryResultVec& results) const;

  void
  getManyByType_(TypeID const& tid,
                 GroupQueryResultVec& results) const;

  int
  getMatchingSequence_(TypeID const& elementType,
                       SelectorBase const& selector,
                       GroupQueryResultVec& results,
                       bool stopIfProcessHasMatch) const;

  EDProductGetter const* prodGetter() const;

  void removeCachedProduct_(ProductID const pid) const;

  //------------------------------------------------------------
  // Data members
  //

  // putProducts_ is the holding pen for EDProducts inserted into this
  // DataViewImpl. Pointers in these collections own the products to
  // which they point.
  TypeLabelMap putProducts_{};

  // gotBranchIDs_ must be mutable because it records all 'gets',
  // which do not logically modify the DataViewImpl. gotBranchIDs_ is
  // merely a cache reflecting what has been retrieved from the
  // Principal class.
  mutable RetrievedProductSet gotBranchIDs_{};

  // Each DataViewImpl must have an associated Principal, used as the
  // source of all 'gets' and the target of 'puts'.
  Principal const& principal_;

  // Each DataViewImpl must have a description of the module executing the
  // "transaction" which the DataViewImpl represents.
  ModuleDescription const& md_;

  // Is this an Event, a SubRun, or a Run.
  BranchType const branchType_;

  // Should we record the parents of the products put into the event.
  bool const recordParents_;
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
  GroupQueryResult bh = get_(TypeID{typeid(PROD)},sel);
  convert_handle(bh, result);
  bool const ok{bh.succeeded() && !result.failedToGet()};
  if (recordParents_ && ok) {
    addToGotBranchIDs(*result.provenance());
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
  GroupQueryResult bh = getByLabel_(TypeID{typeid(PROD)}, label, productInstanceName, processName);
  convert_handle(bh, result);
  bool const ok{bh.succeeded() && !result.failedToGet()};
  if (recordParents_ && ok) {
    addToGotBranchIDs(*result.provenance());
  }
  return ok;
}


template <typename PROD>
inline
PROD const&
art::DataViewImpl::getByLabel(InputTag const& tag) const
{
  art::Handle<PROD> h;
  getByLabel(tag, h);
  return *h;
}

template <typename PROD>
inline
PROD const*
art::DataViewImpl::getPointerByLabel(InputTag const& tag) const
{
  art::Handle<PROD> h;
  getByLabel(tag, h);
  return &(*h);
}

template <typename PROD>
inline
art::ValidHandle<PROD>
art::DataViewImpl::getValidHandle(InputTag const& tag) const
{
  art::Handle<PROD> h;
  getByLabel(tag, h);
  return art::ValidHandle<PROD>(&(*h), *h.provenance());
}

template <typename PROD>
inline
void
art::DataViewImpl::getMany(SelectorBase const& sel,
                           std::vector<Handle<PROD>>& results) const
{
  GroupQueryResultVec bhv;
  getMany_(TypeID{typeid(PROD)}, sel, bhv);

  std::vector<Handle<PROD>> products;
  for (auto const& qr : bhv) {
    Handle<PROD> result;
    convert_handle(qr, result);
    products.push_back(result);
  }
  results.swap(products);

  if (!recordParents_) return;

  for (auto const& h : results)
    addToGotBranchIDs(*h.provenance());
}

template <typename PROD>
inline
void
art::DataViewImpl::getManyByType(std::vector<Handle<PROD>>& results) const
{
  getMany(MatchAllSelector{}, results);
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
