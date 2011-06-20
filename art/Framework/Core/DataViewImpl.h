#ifndef art_Framework_Core_DataViewImpl_h
#define art_Framework_Core_DataViewImpl_h

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
// std::auto_ptr<AppleCollection> pApples( new AppleCollection );
// //fill the collection
// ...
// event.put(pApples);
//
// std::auto_ptr<FruitCollection> pFruits( new FruitCollection );
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

#include "art/Framework/Core/FCPfwd.h"
#include "art/Persistency/Common/BasicHandle.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/OrphanHandle.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Common/fwd.h"
#include "art/Persistency/Common/traits.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/ConstBranchDescription.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "art/Utilities/InputTag.h"
#include "art/Utilities/TypeID.h"

#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

  class DataViewImpl {
  public:
    DataViewImpl(Principal & pcpl,
                 ModuleDescription const& md,
                 BranchType const& branchType);

    ~DataViewImpl();

    size_t size() const;

    template <typename PROD>
    bool
    get(SelectorBase const&, Handle<PROD>& result) const;

    template <typename PROD>
    bool
    getByLabel(std::string const& label, Handle<PROD>& result) const;

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

    ProcessHistory const&
    processHistory() const;

    DataViewImpl const&
    me() const {return *this;}

    typedef std::vector<std::pair<EDProduct*, ConstBranchDescription const *> >  ProductPtrVec;
  protected:

    Principal & principal() {return principal_;}
    Principal const& principal() const {return principal_;}

    ProductPtrVec & putProducts() {return putProducts_;}
    ProductPtrVec const& putProducts() const {return putProducts_;}

    ProductPtrVec & putProductsWithoutParents() {return putProductsWithoutParents_;}
    ProductPtrVec const& putProductsWithoutParents() const {return putProductsWithoutParents_;}

    ConstBranchDescription const&
    getBranchDescription(TypeID const& type, std::string const& productInstanceName) const;

    typedef std::vector<BasicHandle>  BasicHandleVec;

    //------------------------------------------------------------
    // Protected functions.
    //

    // The following 'get' functions serve to isolate the DataViewImpl class
    // from the Principal class.

    BasicHandle
    get_(TypeID const& tid, SelectorBase const&) const;

    BasicHandle
    getByLabel_(TypeID const& tid,
                std::string const& label,
                std::string const& productInstanceName,
                std::string const& processName) const;

    void
    getMany_(TypeID const& tid,
             SelectorBase const& sel,
             BasicHandleVec& results) const;

    void
    getManyByType_(TypeID const& tid,
                   BasicHandleVec& results) const;

    int
    getMatchingSequence_(TypeID const& typeID,
                         SelectorBase const& selector,
                         BasicHandleVec& results,
                         bool stopIfProcessHasMatch) const;

    int
    getMatchingSequenceByLabel_(TypeID const& typeID,
                                std::string const& label,
                                std::string const& productInstanceName,
                                BasicHandleVec& results,
                                bool stopIfProcessHasMatch) const;

    int
    getMatchingSequenceByLabel_(TypeID const& typeID,
                                std::string const& label,
                                std::string const& productInstanceName,
                                std::string const& processName,
                                BasicHandleVec& results,
                                bool stopIfProcessHasMatch) const;

  protected:
    // Also isolates the DataViewImpl class
    // from the Principal class.
    EDProductGetter const* prodGetter() const;
  private:
    //------------------------------------------------------------
    // Copying and assignment of DataViewImpls is disallowed
    //
    DataViewImpl(DataViewImpl const&);                  // not implemented
    DataViewImpl const& operator=(DataViewImpl const&);   // not implemented

  private:
    //------------------------------------------------------------
    // Data members
    //

    // putProducts_ and putProductsWithoutParents_ are the holding
    // pens for EDProducts inserted into this DataViewImpl. Pointers
    // in these collections own the products to which they point.
    //
    ProductPtrVec putProducts_;               // keep parentage info for these
    ProductPtrVec putProductsWithoutParents_; // ... but not for these

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
  operator<<(std::ostream& os, Handle<PROD> const& h)
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
  DataViewImpl::get(SelectorBase const& sel,
                    Handle<PROD>& result) const
  {
    result.clear();
    BasicHandle bh = this->get_(TypeID(typeid(PROD)),sel);
    convert_handle(bh, result);  // throws on conversion error
    if (bh.failedToGet()) {
      return false;
    }
    return true;
  }

  template <typename PROD>
  inline
  bool
  DataViewImpl::getByLabel(std::string const& label,
                           Handle<PROD>& result) const
  {
    result.clear();
    return getByLabel(label, std::string(), result);
  }

  template <typename PROD>
  inline
  bool
  DataViewImpl::getByLabel(InputTag const& tag, Handle<PROD>& result) const
  {
    result.clear();
    BasicHandle bh = this->getByLabel_(TypeID(typeid(PROD)), tag.label(), tag.instance(), tag.process());
    convert_handle(bh, result);  // throws on conversion error
    if (bh.failedToGet()) {
      return false;
    }
    return true;
  }

  template <typename PROD>
  inline
  bool
  DataViewImpl::getByLabel(std::string const& label,
                           std::string const& productInstanceName,
                           Handle<PROD>& result) const
  {
    result.clear();
    BasicHandle bh = this->getByLabel_(TypeID(typeid(PROD)), label, productInstanceName, std::string());
    convert_handle(bh, result);  // throws on conversion error
    if (bh.failedToGet()) {
      return false;
    }
    return true;
  }

  template <typename PROD>
  inline
  void
  DataViewImpl::getMany(SelectorBase const& sel,
                        std::vector<Handle<PROD> >& results) const
  {
    BasicHandleVec bhv;
    this->getMany_(TypeID(typeid(PROD)), sel, bhv);

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

    typename BasicHandleVec::const_iterator it = bhv.begin();
    typename BasicHandleVec::const_iterator end = bhv.end();

    while (it != end) {
      Handle<PROD> result;
      convert_handle(*it, result);  // throws on conversion error
      products.push_back(result);
      ++it;
    }
    results.swap(products);
  }

  template <typename PROD>
  inline
  void
  DataViewImpl::getManyByType(std::vector<Handle<PROD> >& results) const
  {
    BasicHandleVec bhv;
    this->getManyByType_(TypeID(typeid(PROD)), bhv);

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

    typename BasicHandleVec::const_iterator it = bhv.begin();
    typename BasicHandleVec::const_iterator end = bhv.end();

    while (it != end) {
      Handle<PROD> result;
      convert_handle(*it, result);  // throws on conversion error
      products.push_back(result);
      ++it;
    }
    results.swap(products);
  }

}  // art

// ======================================================================

#endif /* art_Framework_Core_DataViewImpl_h */

// Local Variables:
// mode: c++
// End:
