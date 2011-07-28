#ifndef art_Framework_Principal_Event_h
#define art_Framework_Principal_Event_h

// ======================================================================
//
// Event - This is the primary interface for accessing EDProducts from a
//         single collision and inserting new derived products.
//
// For its usage, see "art/Framework/Principal/DataViewImpl.h"
//
// ======================================================================

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Framework/Principal/View.h"
#include "art/Framework/Principal/detail/maybe_record_parents.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Provenance/EventAuxiliary.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Timestamp.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include <cstdlib>
#include <set>
#include <vector>

// ----------------------------------------------------------------------

namespace art {

  class BranchDescription;

  class Event
    : private DataViewImpl
  {
  public:
    typedef DataViewImpl Base;
    Event(EventPrincipal& ep, const ModuleDescription& md);

    // use compiler-generated copy c'tor, copy assignment, and d'tor

    // AUX functions.
    EventID
      id() const {return aux_.id();}
    Timestamp
      time() const {return aux_.time();}
    EventNumber_t
      event() const {return aux_.event();}
    SubRunNumber_t
      subRun() const {return aux_.subRun();}
    RunNumber_t
      run() const {return id().run();}

    bool
      isRealData() const {return aux_.isRealData();}
    EventAuxiliary::ExperimentType
      experimentType() const {return aux_.experimentType();}

    using Base::get;
    using Base::getByLabel;
    using Base::getMany;
    using Base::getManyByType;
    using Base::me;
    using Base::processHistory;
    using Base::size;

    SubRun const& getSubRun() const;

    Run const&
      getRun() const;

    template <typename PROD>
    bool
      get(ProductID const& oid, Handle<PROD>& result) const;

    History const&
      history() const;
    ProcessHistoryID const&
      processHistoryID() const;

    ///Put a new product.
    template <typename PROD>
    ProductID
      put(std::auto_ptr<PROD> product) {return put<PROD>(product, std::string());}

    ///Put a new product with a 'product instance name'
    template <typename PROD>
    ProductID
      put(std::auto_ptr<PROD> product, std::string const& productInstanceName);

    template <typename PROD>
    bool
      get(SelectorBase const& sel,
          Handle<PROD>& result) const;

    template <typename PROD>
    bool
      getByLabel(InputTag const& tag, Handle<PROD>& result) const;

    template <typename PROD>
    bool
      getByLabel(std::string const& label, Handle<PROD>& result) const;

    template <typename PROD>
    bool
      getByLabel(std::string const& label,
                 std::string const& productInstanceName,
                 Handle<PROD>& result) const;

    template <typename PROD>
    void
      getMany(SelectorBase const& sel,
              std::vector<Handle<PROD> >& results) const;

    template <typename PROD>
    void
      getManyByType(std::vector<Handle<PROD> >& results) const;

    template< class ELEMENT >
    std::size_t
      getView( std::string const &            moduleLabel
             , std::vector<ELEMENT const *> & result
             ) const;

    template< class ELEMENT >
    std::size_t
      getView( std::string const &            moduleLabel
             , std::string const &            productInstanceName
             , std::vector<ELEMENT const *> & result
             ) const;

    template< class ELEMENT >
    std::size_t
      getView( InputTag const &               tag
             , std::vector<ELEMENT const *> & result
             ) const;

    // If getView returns true, then result.isValid() is certain to be
    // true -- but the View may still be empty.
    template <class ELEMENT>
    bool
    getView(std::string const& moduleLabel, View<ELEMENT>& result) const;

    template <class ELEMENT>
    bool
    getView(std::string const& moduleLabel, std::string const& instanceName,
            View<ELEMENT>& result) const;

    template <class ELEMENT>
    bool
    getView(InputTag const& tag, View<ELEMENT>& result) const;

    template< typename ELEMENT >
    void
      fillView_( GroupQueryResult & bh
               , std::vector<ELEMENT const *> & result
               ) const;

    // Return true if this Event has been subjected to a process with
    // the given processName, and false otherwise.
    // If true is returned, then ps is filled with the ParameterSet
    // used to configure the identified process.
    bool
      getProcessParameterSet(std::string const& processName,
                             fhicl::ParameterSet& ps) const;

    EDProductGetter const * productGetter() const;

    ProductID branchIDToProductID(BranchID const &bid) const;

  private:
    EventPrincipal const& eventPrincipal() const;
    EventPrincipal      & eventPrincipal();

    ProductID
      makeProductID(BranchDescription const& desc) const;

    void
      ensure_unique_product( std::size_t         nFound
                           , TypeID      const & typeID
                           , std::string const & moduleLabel
                           , std::string const & productInstanceName
                           , std::string const & processName
                           ) const;

    // commit_() is called to complete the transaction represented by
    // this DataViewImpl. The friendships required are gross, but any
    // alternative is not great either.  Putting it into the
    // public interface is asking for trouble
    friend class InputSource;
    friend class DecrepitRelicInputSourceImplementation;
    friend class EDFilter;
    friend class EDProducer;

    void
      commit_();
    void
      commit_aux(Base::ProductPtrVec& products, bool record_parents);

    GroupQueryResult
      getByProductID_(ProductID const& oid) const;

    EventAuxiliary const& aux_;
    std::shared_ptr<SubRun const> const subRun_;

    // gotBranchIDs_ must be mutable because it records all 'gets',
    // which do not logically modify the DataViewImpl. gotBranchIDs_ is
    // merely a cache reflecting what has been retreived from the
    // Principal class.
    typedef std::set<BranchID> BranchIDSet;
    mutable BranchIDSet gotBranchIDs_;
    void
      addToGotBranchIDs(Provenance const& prov) const;
  };  // Event

// ----------------------------------------------------------------------

  template <typename PROD>
  bool
  Event::get(ProductID const& oid, Handle<PROD>& result) const
  {
    result.clear();
    GroupQueryResult bh = this->getByProductID_(oid);
    convert_handle(bh, result);
    if (bh.succeeded())
      addToGotBranchIDs(*result.provenance());
    return bh.succeeded();
  }  // get<>()

// ----------------------------------------------------------------------

  template <typename PROD>
  ProductID
  Event::put(std::auto_ptr<PROD> product, std::string const& productInstanceName)
  {
    if (product.get() == 0) {                // null pointer is illegal
      TypeID typeID(typeid(PROD));
      throw art::Exception(art::errors::NullPointerError)
        << "Event::put: A null auto_ptr was passed to 'put'.\n"
        << "The pointer is of type " << typeID << ".\n"
        << "The specified productInstanceName was '" << productInstanceName << "'.\n";
    }

    BranchDescription const& desc =
      getBranchDescription(TypeID(*product), productInstanceName);

    Wrapper<PROD>* wp(new Wrapper<PROD>(product));

    detail::maybe_record_parents(putProducts(),
                                 putProductsWithoutParents(),
                                 wp,
                                 &desc);
    //    putProducts().push_back(std::make_pair(wp, &desc));

    // product.release(); // The object has been copied into the Wrapper.
    // The old copy must be deleted, so we cannot release ownership.

    return makeProductID(desc);
  }  // put<>()

// ----------------------------------------------------------------------

  template <typename PROD>
  bool
  Event::get(SelectorBase const& sel,
                    Handle<PROD>& result) const
  {
    bool ok = this->Base::get(sel, result);
    if (ok) {
      addToGotBranchIDs(*result.provenance());
    }
    return ok;
  }  // get<>()

// ----------------------------------------------------------------------

  template <typename PROD>
  bool
  Event::getByLabel(InputTag const& tag, Handle<PROD>& result) const
  {
    bool ok = this->Base::getByLabel(tag, result);
    if (ok) {
      addToGotBranchIDs(*result.provenance());
    }
    return ok;
  }  // getByLabel<>()

  template <typename PROD>
  bool
  Event::getByLabel(std::string const& label, Handle<PROD>& result) const
  {
    bool ok = this->Base::getByLabel(label, result);
    if (ok) {
      addToGotBranchIDs(*result.provenance());
    }
    return ok;
  }  // getByLabel<>()

  template <typename PROD>
  bool
  Event::getByLabel(std::string const& label,
                           std::string const& productInstanceName,
                           Handle<PROD>& result) const
  {
    bool ok = this->Base::getByLabel(label, productInstanceName, result);
    if (ok) {
      addToGotBranchIDs(*result.provenance());
    }
    return ok;
  }  // getByLabel<>()

// ----------------------------------------------------------------------

  template <typename PROD>
  void
  Event::getMany(SelectorBase const& sel,
                        std::vector<Handle<PROD> >& results) const
  {
    this->Base::getMany(sel, results);
    for (typename std::vector<Handle<PROD> >::const_iterator it = results.begin(), itEnd = results.end();
        it != itEnd; ++it) {
      addToGotBranchIDs(*it->provenance());
    }
  }  // getMany<>()

// ----------------------------------------------------------------------

  template <typename PROD>
  void
  Event::getManyByType(std::vector<Handle<PROD> >& results) const
  {
    this->Base::getManyByType(results);
    for (typename std::vector<Handle<PROD> >::const_iterator it = results.begin(), itEnd = results.end();
        it != itEnd; ++it) {
      addToGotBranchIDs(*it->provenance());
    }
  }  // getManyByType<>()

// ----------------------------------------------------------------------

  template< class ELEMENT >
  std::size_t
    Event::getView( std::string const &            moduleLabel
                  , std::vector<ELEMENT const *> & result
                  ) const
  {
    return getView(moduleLabel, std::string(), result);
  }

  template< class ELEMENT >
  std::size_t
    Event::getView( std::string const &            moduleLabel
                  , std::string const &            productInstanceName
                  , std::vector<ELEMENT const *> & result
                  ) const
  {
    TypeID typeID( typeid(ELEMENT) );
    GroupQueryResultVec bhv;
    int nFound = getMatchingSequenceByLabel_( typeID
                                            , moduleLabel
                                            , productInstanceName
                                            , bhv
                                            , true
                                            );
    ensure_unique_product( nFound, typeID
                         , moduleLabel, productInstanceName, std::string()
                         );

    std::size_t orig_size = result.size();
    fillView_(bhv[0], result);
    return result.size() - orig_size;
  }  // getView<>()

  template< class ELEMENT >
  std::size_t
    Event::getView( InputTag const &               tag
                  , std::vector<ELEMENT const *> & result
                  ) const
  {
    if (tag.process().empty()) {
      return getView(tag.label(), tag.instance(), result);
    }

    TypeID typeID( typeid(ELEMENT) );
    GroupQueryResultVec bhv;
    int nFound = getMatchingSequenceByLabel_( typeID
                                            , tag.label()
                                            , tag.instance()
                                            , tag.process()
                                            , bhv
                                            , true
                                            );
    ensure_unique_product( nFound, typeID
                         , tag.label(), tag.instance(), tag.process()
                         );


    std::size_t orig_size = result.size();
    fillView_(bhv[0], result);
    return result.size() - orig_size;
  }  // getView<>()

  template <class ELEMENT>
  bool
  Event::getView(std::string const& moduleLabel,
                 View<ELEMENT>&     result) const
  {
    return getView(moduleLabel, std::string(), result);
  }

  template <class ELEMENT>
  bool
  Event::getView(std::string const& moduleLabel,
                 std::string const& productInstanceName,
                 View<ELEMENT>&     result) const
  {
    TypeID typeID( typeid(ELEMENT) );
    GroupQueryResultVec bhv;
    int nFound = getMatchingSequenceByLabel_( typeID
                                            , moduleLabel
                                            , productInstanceName
                                            , bhv
                                            , true
                                            );
    ensure_unique_product( nFound, typeID
                         , moduleLabel, productInstanceName, std::string()
                         );

    fillView_(bhv[0], result.vals());
    result.set_innards(bhv[0].result()->productID(), bhv[0].result()->product());
    return true;
  }

  template <class ELEMENT>
  bool
  Event::getView(InputTag const& tag, View<ELEMENT>& result) const
  {
    if (tag.process().empty())
      return getView(tag.label(), tag.instance(), result);

    TypeID typeID( typeid(ELEMENT) );
    GroupQueryResultVec bhv;
    int nFound = getMatchingSequenceByLabel_(typeID,
                                             tag.label(),
                                             tag.instance(),
                                             tag.process(),
                                             bhv,
                                             true);
    ensure_unique_product( nFound, typeID,
                           tag.label(), tag.instance(), tag.process());

    fillView_(bhv[0], result.vals());
    result.set_innards(bhv[0].result()->productID(), bhv[0].result()->product());
    return true;
  }

// ----------------------------------------------------------------------

  template< typename ELEMENT >
  void
    Event::fillView_( GroupQueryResult & bh
                    , std::vector<ELEMENT const *> & result
                    ) const
  {
    typedef  std::vector<void const *>::const_iterator
             iter_t;

    std::vector<void const *> erased_ptrs;
    bh.result()->product()->fillView(erased_ptrs);
    Handle<ELEMENT> h(bh);
    addToGotBranchIDs(*h.provenance());

    for( iter_t b = erased_ptrs.begin()
              , e = erased_ptrs.end();  b != e;  ++b ) {
      result.push_back( static_cast<ELEMENT const *>(*b) );
    }

  }  // fillView_<>()

}  // art

// ======================================================================

#endif /* art_Framework_Principal_Event_h */

// Local Variables:
// mode: c++
// End:
