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
#include "art/Framework/Principal/detail/maybe_record_parents.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <cstdlib>
#include <memory>
#include <set>
#include <vector>

namespace art {
  class BranchDescription;
  class ProdToProdMapBuilder;  // fwd declaration to avoid circularity
}

class art::Event : private art::DataViewImpl {
public:
  using Base = DataViewImpl;
  Event(EventPrincipal& ep, const ModuleDescription& md);

  // use compiler-generated copy c'tor, copy assignment, and d'tor

  // AUX functions.
  EventID   id() const {return aux_.id();}
  Timestamp time() const {return aux_.time();}

  EventNumber_t  event()  const {return aux_.event(); }
  SubRunNumber_t subRun() const {return aux_.subRun();}
  RunNumber_t    run()    const {return id().run();   }

  bool isRealData() const {return aux_.isRealData();}
  EventAuxiliary::ExperimentType experimentType() const {return aux_.experimentType();}

  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::removeCachedProduct;
  using Base::me;
  using Base::processHistory;
  using Base::size;

  SubRun const& getSubRun() const;

  Run const& getRun() const;

  template <typename PROD>
  bool get(ProductID const& oid, Handle<PROD>& result) const;

  History const& history() const;
  ProcessHistoryID const& processHistoryID() const;

  ///Put a new product.
  template <typename PROD>
  ProductID put(std::unique_ptr<PROD> && product) {return put<PROD>(std::move(product), std::string());}

  ///Put a new product with a 'product instance name'
  template <typename PROD>
  ProductID put(std::unique_ptr<PROD> && product, std::string const& productInstanceName);

  template <typename PROD>
  bool get(SelectorBase const& sel, Handle<PROD>& result) const;

  template <typename PROD>
  PROD const& getByLabel(InputTag const& tag) const;

  template <typename PROD>
  art::ValidHandle<PROD> getValidHandle(InputTag const& tag) const;

  template <typename PROD>
  PROD const* getPointerByLabel(InputTag const& tag) const;

  template <typename PROD>
  bool getByLabel(InputTag const& tag, Handle<PROD>& result) const;

  template <typename PROD>
  bool getByLabel(std::string const& label,
                  std::string const& productInstanceName,
                  Handle<PROD>& result) const;

  template <typename PROD>
  void getMany(SelectorBase const& sel,
               std::vector<Handle<PROD> >& results) const;

  template <typename PROD>
  void getManyByType(std::vector<Handle<PROD> >& results) const;

  // If getView returns true, then result.isValid() is certain to be
  // true -- but the View may still be empty.
  template< class ELEMENT >
  std::size_t getView( std::string const &            moduleLabel,
                       std::string const &            productInstanceName,
                       std::vector<ELEMENT const *> & result) const;

  template< class ELEMENT >
  std::size_t getView( InputTag const &               tag,
                       std::vector<ELEMENT const *> & result) const;

  template <class ELEMENT>
  bool getView(std::string const& moduleLabel, std::string const& instanceName,
               View<ELEMENT>& result) const;

  template <class ELEMENT>
  bool getView(InputTag const& tag, View<ELEMENT>& result) const;

  // Return true if this Event has been subjected to a process with
  // the given processName, and false otherwise.
  // If true is returned, then ps is filled with the ParameterSet
  // used to configure the identified process.
  bool
  getProcessParameterSet(std::string const& processName,
                         fhicl::ParameterSet& ps) const;

  ////    EDProductGetter const * productGetter() const;
  EDProductGetter const * productGetter(ProductID const &) const;

  ProductID branchIDToProductID(BranchID const &bid) const;

  template <typename T>
  using HandleT = Handle<T>;

private:
  EventPrincipal const& eventPrincipal() const { return eventPrincipal_; }
  EventPrincipal      & eventPrincipal()       { return eventPrincipal_; }

  template< typename ELEMENT >
  void fillView_( GroupQueryResult & bh,
                  std::vector<ELEMENT const *> & result) const;

  ProductID makeProductID(BranchDescription const& desc) const;

  void ensure_unique_product( std::size_t         nFound,
                              TypeID      const & typeID,
                              std::string const & moduleLabel,
                              std::string const & productInstanceName,
                              std::string const & processName ) const;

  // commit_() is called to complete the transaction represented by
  // this DataViewImpl. The friendships required are gross, but any
  // alternative is not great either.  Putting it into the
  // public interface is asking for trouble
  friend class InputSource;
  friend class DecrepitRelicInputSourceImplementation;
  friend class EDFilter;
  friend class EDProducer;
  friend class ProdToProdMapBuilder;

  void commit_(bool checkPutProducts,
               ProducedMap const& expectedProducts);
  void commit_aux(Base::BranchIDsMap& products,
                  bool record_parents);

  GroupQueryResult getByProductID_(ProductID const& oid) const;

  EventAuxiliary const& aux_;
  std::shared_ptr<SubRun const> const subRun_;
  EventPrincipal & eventPrincipal_;

  // gotBranchIDs_ must be mutable because it records all 'gets',
  // which do not logically modify the DataViewImpl. gotBranchIDs_ is
  // merely a cache reflecting what has been retreived from the
  // Principal class.
  mutable std::set<BranchID> gotBranchIDs_;
  void addToGotBranchIDs(Provenance const& prov) const;

};  // Event

// ----------------------------------------------------------------------
template <typename PROD>
bool
art::Event::get(ProductID const& oid, Handle<PROD>& result) const
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
art::ProductID
art::Event::put(std::unique_ptr<PROD> && product,
                std::string const& productInstanceName)
{
  if (product.get() == nullptr) {
    throw art::Exception(art::errors::NullPointerError)
      << "Event::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << TypeID(typeid(PROD)) << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  auto const& bd = getBranchDescription(TypeID(*product), productInstanceName);
  auto        wp = std::make_unique<Wrapper<PROD>>(std::move(product));

  auto result = detail::maybe_record_parents(putProducts(),
                                             putProductsWithoutParents(),
                                             std::move(wp),
                                             bd);
  if ( !result.second ) {
    throw art::Exception(art::errors::InsertFailure)
      << "Event::put: Attempt to put multiple products with the\n"
      << "            following description onto the Event.\n"
      << "            Products must be unique per Event.\n"
      << "=================================\n"
      << bd
      << "=================================\n";
  }

  return makeProductID(bd);
}  // put<>()

// ----------------------------------------------------------------------

template <typename PROD>
bool
art::Event::get(SelectorBase const& sel,
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
PROD const&
art::Event::getByLabel(InputTag const& tag) const
{
  art::Handle<PROD> h;
  getByLabel(tag, h);
  return *h;
}

template <typename PROD>
art::ValidHandle<PROD>
art::Event::getValidHandle(InputTag const& tag) const
{
  art::Handle<PROD> h;
  getByLabel(tag, h);
  return art::ValidHandle<PROD>(&(*h), *h.provenance());
}

template <typename PROD>
PROD const*
art::Event::getPointerByLabel(InputTag const& tag) const
{
  art::Handle<PROD> h;
  getByLabel(tag, h);
  return &(*h);
}

template <typename PROD>
bool
art::Event::getByLabel(InputTag const& tag, Handle<PROD>& result) const
{
  bool ok = this->Base::getByLabel(tag, result);
  if (ok) {
    addToGotBranchIDs(*result.provenance());
  }
  return ok;
}  // getByLabel<>()

template <typename PROD>
bool
art::Event::getByLabel(std::string const& label,
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
art::Event::getMany(SelectorBase const& sel,
               std::vector<Handle<PROD> >& results) const
{
  this->Base::getMany(sel, results);
  for (auto const& h : results)
    addToGotBranchIDs(*h.provenance());
}  // getMany<>()

// ----------------------------------------------------------------------

template <typename PROD>
void
art::Event::getManyByType(std::vector<Handle<PROD> >& results) const
{
  this->Base::getManyByType(results);
  for (auto const& h : results)
    addToGotBranchIDs(*h.provenance());
}  // getManyByType<>()


template< class ELEMENT >
std::size_t
art::Event::getView(std::string const & moduleLabel,
                    std::string const & productInstanceName,
                    std::vector<ELEMENT const *> & result) const
{
  TypeID typeID( typeid(ELEMENT) );
  GroupQueryResultVec bhv;
  int nFound = getMatchingSequenceByLabel_( typeID,
                                            moduleLabel,
                                            productInstanceName,
                                            bhv,
                                            true );
  ensure_unique_product( nFound, typeID,
                         moduleLabel, productInstanceName, std::string() );

  std::size_t const orig_size = result.size();
  fillView_(bhv[0], result);
  return result.size() - orig_size;
}  // getView<>()

template< class ELEMENT >
std::size_t
art::Event::getView( InputTag const & tag,
                     std::vector<ELEMENT const *> & result ) const
{
  if (tag.process().empty()) {
    return getView(tag.label(), tag.instance(), result);
  }

  TypeID typeID( typeid(ELEMENT) );
  GroupQueryResultVec bhv;
  int nFound = getMatchingSequenceByLabel_( typeID,
                                            tag.label(),
                                            tag.instance(),
                                            tag.process(),
                                            bhv,
                                            true );
  ensure_unique_product( nFound, typeID,
                         tag.label(), tag.instance(), tag.process() );

  std::size_t const orig_size = result.size();
  fillView_(bhv[0], result);
  return result.size() - orig_size;
}  // getView<>()

template <class ELEMENT>
bool
art::Event::getView(std::string const& moduleLabel,
               std::string const& productInstanceName,
               View<ELEMENT>&     result) const
{
  TypeID typeID( typeid(ELEMENT) );
  GroupQueryResultVec bhv;
  int nFound = getMatchingSequenceByLabel_( typeID,
                                            moduleLabel,
                                            productInstanceName,
                                            bhv,
                                            true );
  ensure_unique_product( nFound, typeID,
                         moduleLabel, productInstanceName, std::string() );

  fillView_(bhv[0], result.vals());
  result.set_innards(bhv[0].result()->productID(), bhv[0].result()->uniqueProduct());
  return true;
}

template <class ELEMENT>
bool
art::Event::getView(InputTag const& tag, View<ELEMENT>& result) const
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
  result.set_innards(bhv[0].result()->productID(), bhv[0].result()->uniqueProduct());
  return true;
}

// ----------------------------------------------------------------------

template< typename ELEMENT >
void
art::Event::fillView_( GroupQueryResult & bh,
                       std::vector<ELEMENT const*> & result ) const
{
  std::vector<void const *> erased_ptrs;
  bh.result()->uniqueProduct()->fillView(erased_ptrs);
  addToGotBranchIDs(Provenance(bh.result()));

  std::vector<ELEMENT const*> vals;
  cet::transform_all( erased_ptrs,
                      std::back_inserter(vals),
                      [](auto p) {
                        return static_cast<ELEMENT const*>(p);
                      } );

  result.swap(vals);
}

#endif /* art_Framework_Principal_Event_h */

// Local Variables:
// mode: c++
// End:
