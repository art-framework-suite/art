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
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Provenance/detail/type_aliases.h"
#include "art/Utilities/HorizontalRule.h"
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

class art::Event final : private art::DataViewImpl {
public:

  using Base = DataViewImpl;
  explicit Event(EventPrincipal const& ep,
                 ModuleDescription const& md);

  // AUX functions.
  EventID   id() const {return aux_.id();}
  Timestamp time() const {return aux_.time();}

  EventNumber_t  event()  const {return aux_.event(); }
  SubRunNumber_t subRun() const {return aux_.subRun();}
  RunNumber_t    run()    const {return id().run();   }

  bool isRealData() const {return aux_.isRealData();}
  EventAuxiliary::ExperimentType experimentType() const {return aux_.experimentType();}

  SubRun const& getSubRun() const;
  Run const& getRun() const;

  History const& history() const;
  ProcessHistoryID const& processHistoryID() const;

  // Put a new product.
  template <typename PROD>
  ProductID put(std::unique_ptr<PROD>&& product) {return put<PROD>(std::move(product), std::string());}

  // Put a new product with a 'product instance name'
  template <typename PROD>
  ProductID put(std::unique_ptr<PROD>&& product, std::string const& productInstanceName);

  // Retrieve a product
  using Base::get;
  using Base::getByLabel;
  using Base::getMany;
  using Base::getManyByType;
  using Base::getPointerByLabel;
  using Base::getValidHandle;

  // Only Event products have a notion of ProductID.
  template <typename PROD>
  bool get(ProductID const& oid, Handle<PROD>& result) const;

  // If getView returns true, then result.isValid() is certain to be
  // true -- but the View may still be empty.
  template <typename ELEMENT>
  std::size_t getView(std::string const& moduleLabel,
                      std::string const& productInstanceName,
                      std::vector<ELEMENT const*>& result) const;

  template <typename ELEMENT>
  std::size_t getView(InputTag const& tag,
                      std::vector<ELEMENT const*>& result) const;

  template <typename ELEMENT>
  bool getView(std::string const& moduleLabel,
               std::string const& instanceName,
               View<ELEMENT>& result) const;

  template <typename ELEMENT>
  bool getView(InputTag const& tag, View<ELEMENT>& result) const;

  // Expert-level
  using Base::removeCachedProduct;
  using Base::processHistory;
  using Base::size;

  // Return true if this Event has been subjected to a process with
  // the given processName, and false otherwise.
  // If true is returned, then ps is filled with the ParameterSet
  // used to configure the identified process.
  bool
  getProcessParameterSet(std::string const& processName,
                         fhicl::ParameterSet& ps) const;

  EDProductGetter const* productGetter(ProductID const&) const;

  template <typename T>
  using HandleT = Handle<T>;

private:

  using Base::addToGotBranchIDs;

  template <typename ELEMENT>
  void fillView_(GroupQueryResult& bh,
                 std::vector<ELEMENT const*>& result) const;

  ProductID makeProductID(BranchDescription const& desc) const;

  void ensure_unique_product(std::size_t nFound,
                             TypeID const& typeID,
                             std::string const& moduleLabel,
                             std::string const& productInstanceName,
                             std::string const& processName) const;

  // commit_() is called to complete the transaction represented by
  // this DataViewImpl. The friendships required are gross, but any
  // alternative is not great either.  Putting it into the public
  // interface is asking for trouble
  friend class InputSource;
  friend class DecrepitRelicInputSourceImplementation;
  friend class EDFilter;
  friend class EDProducer;
  friend class ProdToProdMapBuilder;

  void commit_(EventPrincipal&,
               bool checkPutProducts,
               std::set<TypeLabel> const& expectedProducts);

  GroupQueryResult getByProductID_(ProductID const& oid) const;

  EventAuxiliary const& aux_;
  std::unique_ptr<SubRun const> const subRun_;
  EventPrincipal const& eventPrincipal_;

  // FIXME: The 'recordParents_' flag is necessary only for Event::get
  // overload below.  For all other lookups, the
  // DataViewImpl::recordParents_ flag is sufficient.  However, since
  // a ProductID does not have meaning for a (Sub)Run product (at this
  // point), we save the record-parents flag here too.
  bool const recordParents_;
};  // Event

// ----------------------------------------------------------------------

template <typename PROD>
art::ProductID
art::Event::put(std::unique_ptr<PROD>&& product,
                std::string const& productInstanceName)
{
  TypeID const tid{typeid(PROD)};
  if (product.get() == nullptr) {
    throw art::Exception(art::errors::NullPointerError)
      << "Event::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << tid << ".\n"
      << "The specified productInstanceName was '" << productInstanceName << "'.\n";
  }

  auto const& bd = getBranchDescription(tid, productInstanceName);
  auto wp = std::make_unique<Wrapper<PROD>>(std::move(product));

  auto result = putProducts().emplace(TypeLabel{tid, productInstanceName},
                                      DataViewImpl::PMValue{std::move(wp), bd, RangeSet::invalid()});
  if (!result.second) {
    HorizontalRule rule{30};
    throw art::Exception(art::errors::ProductPutFailure)
      << "Event::put: Attempt to put multiple products with the\n"
      << "            following description onto the Event.\n"
      << "            Products must be unique per Event.\n"
      << rule('=') << '\n'
      << bd
      << rule('=') << '\n';
  }

  return makeProductID(bd);
}  // put<>()

// ----------------------------------------------------------------------
template <typename PROD>
bool
art::Event::get(ProductID const& oid, Handle<PROD>& result) const
{
  result.clear(); // Is this the correct thing to do if an exception is thrown?
  GroupQueryResult bh = getByProductID_(oid);
  convert_handle(bh, result);
  bool const ok{bh.succeeded() && !result.failedToGet()};
  if (recordParents_ && ok) {
    addToGotBranchIDs(*result.provenance());
  }
  return ok;
}  // get<>()

// ----------------------------------------------------------------------

template <typename ELEMENT>
std::size_t
art::Event::getView(std::string const& moduleLabel,
                    std::string const& productInstanceName,
                    std::vector<ELEMENT const*>& result) const
{
  TypeID const typeID{typeid(ELEMENT)};
  GroupQueryResultVec bhv;
  int const nFound = getMatchingSequenceByLabel_(typeID,
                                                 moduleLabel,
                                                 productInstanceName,
                                                 bhv,
                                                 true);
  ensure_unique_product(nFound, typeID,
                        moduleLabel, productInstanceName, std::string());

  std::size_t const orig_size = result.size();
  fillView_(bhv[0], result);
  return result.size() - orig_size;
}  // getView<>()

template <typename ELEMENT>
std::size_t
art::Event::getView(InputTag const& tag,
                    std::vector<ELEMENT const*>& result) const
{
  if (tag.process().empty()) {
    return getView(tag.label(), tag.instance(), result);
  }

  TypeID const typeID{typeid(ELEMENT)};
  GroupQueryResultVec bhv;
  int const nFound = getMatchingSequenceByLabel_(typeID,
                                                 tag.label(),
                                                 tag.instance(),
                                                 tag.process(),
                                                 bhv,
                                                 true);
  ensure_unique_product(nFound, typeID,
                        tag.label(), tag.instance(), tag.process());

  std::size_t const orig_size = result.size();
  fillView_(bhv[0], result);
  return result.size() - orig_size;
}  // getView<>()

template <typename ELEMENT>
bool
art::Event::getView(std::string const& moduleLabel,
                    std::string const& productInstanceName,
                    View<ELEMENT>& result) const
{
  TypeID const typeID{typeid(ELEMENT)};
  GroupQueryResultVec bhv;
  int const nFound = getMatchingSequenceByLabel_(typeID,
                                                 moduleLabel,
                                                 productInstanceName,
                                                 bhv,
                                                 true);
  ensure_unique_product(nFound, typeID,
                        moduleLabel, productInstanceName, std::string());

  fillView_(bhv[0], result.vals());
  result.set_innards(bhv[0].result()->productID(), bhv[0].result()->uniqueProduct());
  return true;
}

template <typename ELEMENT>
bool
art::Event::getView(InputTag const& tag, View<ELEMENT>& result) const
{
  if (tag.process().empty()) {
    return getView(tag.label(), tag.instance(), result);
  }

  TypeID const typeID{typeid(ELEMENT)};
  GroupQueryResultVec bhv;
  int const nFound = getMatchingSequenceByLabel_(typeID,
                                                 tag.label(),
                                                 tag.instance(),
                                                 tag.process(),
                                                 bhv,
                                                 true);
  ensure_unique_product(nFound, typeID,
                        tag.label(), tag.instance(), tag.process());

  fillView_(bhv[0], result.vals());
  result.set_innards(bhv[0].result()->productID(), bhv[0].result()->uniqueProduct());
  return true;
}

// ----------------------------------------------------------------------

template <typename ELEMENT>
void
art::Event::fillView_(GroupQueryResult& bh,
                      std::vector<ELEMENT const*>& result) const
{
  std::vector<void const*> erased_ptrs;
  bh.result()->uniqueProduct()->fillView(erased_ptrs);
  addToGotBranchIDs(Provenance{bh.result()});

  std::vector<ELEMENT const*> vals;
  cet::transform_all(erased_ptrs,
                     std::back_inserter(vals),
                     [](auto p) {
                       return static_cast<ELEMENT const*>(p);
                     });

  result.swap(vals);
}

#endif /* art_Framework_Principal_Event_h */

// Local Variables:
// mode: c++
// End:
