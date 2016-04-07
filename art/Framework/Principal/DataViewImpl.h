#ifndef art_Framework_Principal_DataViewImpl_h
#define art_Framework_Principal_DataViewImpl_h

// ======================================================================
//
// DataViewImpl - This is the implementation for accessing EDProducts and
// inserting new EDproducts.
//
// Getting Data
//
// The art::DataViewImpl class provides many 'get*" methods for getting
// data it contains.
//
// The primary method for getting data is to use getByLabel(). The labels
// are the label of the module assigned in the configuration file and the
// 'product instance label' (which can be omitted in the case the 'product
// instance label' is the default value).  The C++ type of the product
// plus the two labels uniquely identify a product in the DataViewImpl.
//
// We use an event in the examples, but a run or a subrun can also hold
// products.
//
// art::Handle<AppleCollection> apples;
// event.getByLabel("tree",apples);
//
// art::Handle<FruitCollection> fruits;
// event.getByLabel("market", "apple", fruits);
//
// Putting Data
//
// std::unique_ptr<AppleCollection> pApples( new AppleCollection );
// //fill the collection
// ...
// event.put(std::move(pApples));
//
// std::unique_ptr<FruitCollection> pFruits( new FruitCollection );
// //fill the collection
// ...
// event.put("apple", pFruits);
//
// //do loop and fill collection
// for(unsigned int index = 0; ..... ) {
// ....
// apples->push_back( Apple(...) );
//
// }
//
// ======================================================================

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "art/Persistency/Common/fwd.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchID.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
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
  DataViewImpl(Principal & pcpl,
               ModuleDescription const& md,
               BranchType const& branchType);

  size_t size() const;

  template <typename PROD>
  bool
  get(SelectorBase const&, Handle<PROD>& result) const;

  template <typename PROD>
  bool
  getByLabel(std::string const& label,
             std::string const& productInstanceName,
             Handle<PROD>& result) const;

  /// same as above, but using the InputTag class
  template <typename PROD>
  bool
  getByLabel(InputTag const& tag, Handle<PROD>& result) const;

  template <typename PROD>
  void
  getMany(SelectorBase const&, std::vector<Handle<PROD> >& results) const;

  template <typename PROD>
  void
  getManyByType(std::vector<Handle<PROD> >& results) const;

  template <typename PROD>
  bool removeCachedProduct(Handle<PROD> & h) const;

  ProcessHistory const&
  processHistory() const;

  DataViewImpl const&
  me() const {return *this;}

  struct PMValue {

    PMValue( std::unique_ptr<EDProduct>&& p, BranchDescription const& b )
      : prod{std::move(p)}, bd{b}
    {}

    std::unique_ptr<EDProduct> prod;
    BranchDescription const& bd;
  };

  using BranchIDsMap = std::unordered_map<BranchID, PMValue, BranchID::Hash>;

protected:

  Principal      & principal()       {return principal_;}
  Principal const& principal() const {return principal_;}

  BranchIDsMap      & putProducts()       {return putProducts_;}
  BranchIDsMap const& putProducts() const {return putProducts_;}

  BranchIDsMap      & putProductsWithoutParents()       {return putProductsWithoutParents_;}
  BranchIDsMap const& putProductsWithoutParents() const {return putProductsWithoutParents_;}

  void
  checkPutProducts(bool checkProducts,
                   ProducedMap const& expectedBids,
                   BranchIDsMap const& products );

  BranchDescription const&
  getBranchDescription(TypeID const& type, std::string const& productInstanceName) const;

  typedef std::vector<GroupQueryResult>  GroupQueryResultVec;

  //------------------------------------------------------------
  // Protected functions.
  //

  // The following 'get' functions serve to isolate the DataViewImpl class
  // from the Principal class.

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

protected:
  // Also isolates the DataViewImpl class
  // from the Principal class.
  EDProductGetter const* prodGetter() const;

private:
  void removeCachedProduct_(BranchID const & bid) const;

  //------------------------------------------------------------
  // Data members
  //

  // putProducts_ and putProductsWithoutParents_ are the holding
  // pens for EDProducts inserted into this DataViewImpl. Pointers
  // in these collections own the products to which they point.
  //
  BranchIDsMap putProducts_;               // keep parentage info for these
  BranchIDsMap putProductsWithoutParents_; // ... but not for these

  // Each DataViewImpl must have an associated Principal, used as the
  // source of all 'gets' and the target of 'puts'.
  Principal & principal_;

  // Each DataViewImpl must have a description of the module executing the
  // "transaction" which the DataViewImpl represents.
  ModuleDescription const& md_;

  // Is this an Event, a SubRun, or a Run.
  BranchType const branchType_;
};

template <typename PROD>
inline
std::ostream&
art::operator<<(std::ostream& os, Handle<PROD> const& h)
{
  os << h.product() << " " << h.provenance() << " " << h.id();
  return os;
}

// Implementation of  DataViewImpl  member templates. See  DataViewImpl.cc for the
// implementation of non-template members.
//

template <typename PROD>
inline
bool
art::DataViewImpl::get(SelectorBase const& sel,
                       Handle<PROD>& result) const
{
  result.clear();
  GroupQueryResult bh = get_(TypeID(typeid(PROD)),sel);
  convert_handle(bh, result);
  return bh.succeeded();
}

template <typename PROD>
inline
bool
art::DataViewImpl::getByLabel(InputTag const& tag, Handle<PROD>& result) const
{
  result.clear();
  GroupQueryResult bh = getByLabel_(TypeID(typeid(PROD)), tag.label(), tag.instance(), tag.process());
  convert_handle(bh, result);
  return bh.succeeded();
}

template <typename PROD>
inline
bool
art::DataViewImpl::getByLabel(std::string const& label,
                              std::string const& productInstanceName,
                              Handle<PROD>& result) const
{
  result.clear();
  GroupQueryResult bh = getByLabel_(TypeID(typeid(PROD)), label, productInstanceName, std::string());
  convert_handle(bh, result);
  return bh.succeeded();
}

template <typename PROD>
inline
void
art::DataViewImpl::getMany(SelectorBase const& sel,
                           std::vector<Handle<PROD>>& results) const
{
  GroupQueryResultVec bhv;
  getMany_(TypeID(typeid(PROD)), sel, bhv);

  // Go through the returned handles; for each element,
  //   1. create a Handle<PROD> and
  //
  // This function presents an exception safety difficulty. If an
  // exception is thrown when converting a handle, the "got
  // products" record will be wrong.
  //
  // Since EDProducers are not allowed to use this function,
  // the problem does not seem too severe.
  //
  // Question: do we even need to keep track of the "got products"
  // for this function, since it is *not* to be used by EDProducers?
  std::vector<Handle<PROD> > products;

  for (auto const& qr : bhv) {
    Handle<PROD> result;
    convert_handle(qr, result);  // throws on conversion error
    products.push_back(result);
  }
  results.swap(products);
}

template <typename PROD>
inline
void
art::DataViewImpl::getManyByType(std::vector<Handle<PROD>>& results) const
{
  GroupQueryResultVec bhv;
  getManyByType_(TypeID(typeid(PROD)), bhv);

  // Go through the returned handles; for each element,
  //   1. create a Handle<PROD> and
  //
  // This function presents an exception safety difficulty. If an
  // exception is thrown when converting a handle, the "got
  // products" record will be wrong.
  //
  // Since EDProducers are not allowed to use this function,
  // the problem does not seem too severe.
  //
  // Question: do we even need to keep track of the "got products"
  // for this function, since it is *not* to be used by EDProducers?
  std::vector<Handle<PROD> > products;

  for (auto const& qr : bhv) {
    Handle<PROD> result;
    convert_handle(qr, result);  // throws on conversion error
    products.push_back(result);
  }
  results.swap(products);
}

template <typename PROD>
bool
art::DataViewImpl::
removeCachedProduct(Handle<PROD> & h) const {
  bool result { false };
  if (h.isValid() && !h.provenance()->produced()) {
    removeCachedProduct_(h.provenance()->branchID());
    h.clear();
    result = true;
  }
  return result;
}

#endif /* art_Framework_Principal_DataViewImpl_h */

// Local Variables:
// mode: c++
// End:
