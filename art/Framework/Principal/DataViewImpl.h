#ifndef art_Framework_Principal_DataViewImpl_h
#define art_Framework_Principal_DataViewImpl_h
// vim: set sw=2 expandtab :

//
//  DataViewImpl - This is the implementation for accessing EDProducts
//  and inserting new EDproducts.
//
//  Getting Data
//
//  The art::DataViewImpl class provides many 'get*" methods for
//  getting data it contains.
//
//  The primary method for getting data is to use getByLabel(). The
//  labels are the label of the module assigned in the configuration
//  file and the 'product instance label' (which can be omitted in the
//  case the 'product instance label' is the default value).  The C++
//  type of the product plus the two labels uniquely identify a product
//  in the DataViewImpl.
//
//  We use an Event in the examples, but a Run or a SubRun can also
//  hold products.
//
//    art::Handle<AppleCollection> apples;
//    event.getByLabel("tree", apples);
//
//    art::Handle<FruitCollection> fruits;
//    event.getByLabel("market", "apples", fruits);
//
//  Putting Data
//
//    auto pApples = std::make_unique<AppleCollection>();
//    // fill the collection
//    ...
//    event.put(std::move(pApples));
//
//    auto pFruits = std::make_unique<FruitCollection>();
//    // fill the collection
//    ...
//    event.put(std::move(pFruits), "apples");
//

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Common/fwd.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/ProductSemantics.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Common/detail/maybeCastObj.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Persistency/Provenance/type_aliases.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "canvas/Utilities/WrappedTypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {

  class EDProductGetter;

  template <typename PROD>
  std::ostream& operator<<(std::ostream& os, Handle<PROD> const& h);

  class DataViewImpl {

    // Give access to commit(...).
    friend class EDFilter;
    friend class EDProducer;
    friend class ResultsProducer;
    friend class ProducingService;

  public: // TYPES
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

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~DataViewImpl();

    explicit DataViewImpl(BranchType bt,
                          Principal const& p,
                          ModuleDescription const& md,
                          bool recordParents,
                          TypeLabelLookup_t const& expectedProducts,
                          RangeSet const& rs = RangeSet::invalid());

    DataViewImpl(DataViewImpl const&) = delete;
    DataViewImpl(DataViewImpl&&) = delete;
    DataViewImpl& operator=(DataViewImpl const&) = delete;
    DataViewImpl& operator=(DataViewImpl&) = delete;

  public: // MEMBER FUNCTIONS -- User-facing API - misc
    RunID runID() const;

    SubRunID subRunID() const;

    EventID eventID() const;

    RunNumber_t run() const;

    SubRunNumber_t subRun() const;

    EventNumber_t event() const;

    Timestamp const& beginTime() const;

    Timestamp const& endTime() const;

    Timestamp time() const;

    bool isRealData() const;

    EventAuxiliary::ExperimentType experimentType() const;

    History const& history() const;

    ProcessHistoryID const& processHistoryID() const;

    size_t size() const;

    ProcessHistory const& processHistory() const;

    template <typename PROD>
    bool removeCachedProduct(Handle<PROD>&) const;

    EDProductGetter const* productGetter(ProductID const pid) const;

    bool getProcessParameterSet(std::string const& process,
                                fhicl::ParameterSet&) const;

  public: // MEMBER FUNCTIONS -- User-facing API -- get*
    template <typename PROD>
    bool get(SelectorBase const&, Handle<PROD>& result) const;

    template <typename PROD>
    bool get(ProductID const pid, Handle<PROD>& result) const;

    template <typename PROD>
    bool getByLabel(std::string const& label,
                    std::string const& instance,
                    Handle<PROD>& result) const;

    template <typename PROD>
    bool getByLabel(std::string const& label,
                    std::string const& instance,
                    std::string const& process,
                    Handle<PROD>& result) const;

  public: // MEMBER FUNCTIONS -- User-facing API -- get*, using InputTag
    template <typename PROD>
    PROD const& getByLabel(InputTag const& tag) const;

    template <typename PROD>
    bool getByLabel(InputTag const& tag, Handle<PROD>& result) const;

    template <typename PROD>
    PROD const* getPointerByLabel(InputTag const& tag) const;

    template <typename PROD>
    ValidHandle<PROD> getValidHandle(InputTag const& tag) const;

    template <typename PROD>
    bool getByToken(ProductToken<PROD> const&, Handle<PROD>& result) const;

    template <typename PROD>
    ValidHandle<PROD> getValidHandle(ProductToken<PROD> const&) const;

  public: // MEMBER FUNCTIONS -- User-facing API -- getMany*
    template <typename PROD>
    void getMany(SelectorBase const&, std::vector<Handle<PROD>>& results) const;

    template <typename PROD>
    void getManyByType(std::vector<Handle<PROD>>& results) const;

  public: // MEMBER FUNCTIONS -- User-facing API -- getView
    // If getView returns true, then result.isValid() is true, but the View may
    // still be empty.
    template <typename ELEMENT>
    bool getView(std::string const& label,
                 std::string const& instance,
                 View<ELEMENT>& result) const;

    template <typename ELEMENT>
    std::size_t getView(std::string const& label,
                        std::string const& instance,
                        std::vector<ELEMENT const*>& result) const;

    // If getView returns true, then result.isValid() is true, but the View may
    // still be empty.
    template <typename ELEMENT>
    bool getView(InputTag const&, View<ELEMENT>& result) const;

    template <typename ELEMENT>
    std::size_t getView(InputTag const&,
                        std::vector<ELEMENT const*>& result) const;

    // FIXME: Implementation missing!
    template <typename ELEMENT>
    std::size_t getView(ViewToken<ELEMENT> const&,
                        std::vector<ELEMENT const*>& result) const;

    template <typename ELEMENT>
    bool getView(ViewToken<ELEMENT> const&, View<ELEMENT>& result) const;

    template <typename T>
    ProductID getProductID() const;

    template <typename T>
    ProductID getProductID(std::string const& instance_name) const;

  public: // MEMBER FUNCTIONS -- User-facing API -- put*
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FullSemantic<Level::Run> const semantic);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FragmentSemantic<Level::Run> const semantic);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  RangedFragmentSemantic<Level::Run> semantic);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  FullSemantic<Level::Run>);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  FragmentSemantic<Level::Run>);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  RangedFragmentSemantic<Level::Run> semantic);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FullSemantic<Level::SubRun> const semantic);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FragmentSemantic<Level::SubRun> const semantic);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  RangedFragmentSemantic<Level::SubRun> semantic);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  FullSemantic<Level::SubRun>);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  FragmentSemantic<Level::SubRun>);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  RangedFragmentSemantic<Level::SubRun> semantic);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp, std::string const& instance);

    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  RangeSet const& rs);

  private: // MEMBER FUNCTIONS -- implementation details
    using GroupQueryResultVec = std::vector<GroupQueryResult>;

    void removeNonViewableMatches_(TypeID const& requestedElementType,
                                   GroupQueryResultVec& results) const;

    void ensureUniqueProduct_(std::size_t nFound,
                              TypeID const& typeID,
                              std::string const& moduleLabel,
                              std::string const& productInstanceName,
                              std::string const& processName) const;

    // The following 'get' functions serve to isolate the DataViewImpl class
    // from the Principal class.
    GroupQueryResult get_(WrappedTypeID const& wrapped,
                          SelectorBase const&) const;

    GroupQueryResult getByProductID_(ProductID const pid) const;

    void removeCachedProduct_(ProductID const pid) const;

    template <typename ELEMENT>
    void fillView_(GroupQueryResult& bh,
                   std::vector<ELEMENT const*>& result) const;

    void checkPutProducts();

    BranchDescription const& getProductDescription_(
      TypeLabel const& typeLabel) const;

    void recordAsParent(Provenance const& prov) const;

    ProductID getProductID_(TypeID const& type,
                            std::string const& instance) const;

    GroupQueryResult getByLabel_(WrappedTypeID const& wrapped,
                                 std::string const& label,
                                 std::string const& productInstanceName,
                                 std::string const& processName) const;

    GroupQueryResultVec getMany_(WrappedTypeID const& wrapped,
                                 SelectorBase const& sel) const;

    GroupQueryResultVec getMatchingSequenceByLabel_(
      std::string const& label,
      std::string const& productInstanceName,
      std::string const& processName) const;

    // If getView returns true, then result.isValid() is certain to be
    // true -- but the View may still be empty.
    template <typename ELEMENT>
    GroupQueryResultVec getView_(std::string const& moduleLabel,
                                 std::string const& productInstanceName,
                                 std::string const& processName) const;

    void commit(Principal& principal, bool const checkProducts);

    void commit(Principal& principal);

  private: // MEMBER DATA
    // Is this an Event, a Run, a SubRun, or a Results.
    BranchType const branchType_;

    // Each DataViewImpl must have an associated Principal, used as the
    // source of all 'gets' and the target of 'puts'.
    Principal const& principal_;

    // Each DataViewImpl must have a description of the module executing the
    // "transaction" which the DataViewImpl represents.
    ModuleDescription const& md_;

    // Should we record the parents of the products put into the event.
    bool const recordParents_;

    // must be mutable because it records all 'gets',
    // which do not logically modify the DataViewImpl. retrievedProducts_ is
    // merely a cache reflecting what has been retrieved from the
    // Principal class.
    mutable std::set<ProductID> retrievedProducts_{};

    TypeLabelLookup_t const& expectedProducts_;

    // holding pen for EDProducts inserted into this
    // DataViewImpl. Pointers in these collections own the products to
    // which they point.
    std::map<TypeLabel, PMValue> putProducts_{};

    RangeSet rangeSet_{RangeSet::invalid()};
  };

  template <typename PROD>
  inline std::ostream&
  operator<<(std::ostream& os, Handle<PROD> const& h)
  {
    os << h.product() << " " << h.provenance() << " " << h.id();
    return os;
  }

} // namespace art

template <typename PROD>
inline bool
art::DataViewImpl::get(SelectorBase const& sel, Handle<PROD>& result) const
{
  result.clear(); // Is this the correct thing to do if an exception is thrown?
  // We do *not* track whether consumes was called for a SelectorBase.
  GroupQueryResult bh = get_(WrappedTypeID::make<PROD>(), sel);
  convert_handle(bh, result);
  bool const ok = bh.succeeded() && !result.failedToGet();
  if (recordParents_ && ok) {
    recordAsParent(*result.provenance());
  }
  return ok;
}

template <typename PROD>
bool
art::DataViewImpl::get(ProductID const pid, Handle<PROD>& result) const
{
  // Is this the correct thing to do if an exception is thrown?
  result.clear();
  GroupQueryResult bh = getByProductID_(pid);
  convert_handle(bh, result);
  bool const ok = bh.succeeded() && !result.failedToGet();
  if (recordParents_ && ok) {
    recordAsParent(*result.provenance());
  }
  return ok;
}

template <typename PROD>
inline bool
art::DataViewImpl::getByLabel(InputTag const& tag, Handle<PROD>& result) const
{
  return getByLabel<PROD>(tag.label(), tag.instance(), tag.process(), result);
}

template <typename PROD>
inline bool
art::DataViewImpl::getByToken(ProductToken<PROD> const& token,
                              Handle<PROD>& result) const
{
  auto const& tag = token.inputTag_;
  return getByLabel<PROD>(tag.label(), tag.instance(), tag.process(), result);
}

template <typename PROD>
inline bool
art::DataViewImpl::getByLabel(std::string const& label,
                              std::string const& instance,
                              Handle<PROD>& result) const
{
  return getByLabel<PROD>(label, instance, {}, result);
}

template <typename PROD>
inline bool
art::DataViewImpl::getByLabel(std::string const& label,
                              std::string const& productInstanceName,
                              std::string const& processName,
                              Handle<PROD>& result) const
{
  result.clear(); // Is this the correct thing to do if an exception is thrown?
  auto const wrapped = WrappedTypeID::make<PROD>();
  ProductInfo const pinfo{ProductInfo::ConsumableType::Product,
                          wrapped.product_type,
                          label,
                          productInstanceName,
                          processName};
  ConsumesInfo::instance()->validateConsumedProduct(branchType_, md_, pinfo);
  GroupQueryResult bh =
    getByLabel_(wrapped, label, productInstanceName, processName);
  convert_handle(bh, result);
  bool const ok = bh.succeeded() && !result.failedToGet();
  if (recordParents_ && ok) {
    recordAsParent(*result.provenance());
  }
  return ok;
}

template <typename PROD>
inline PROD const&
art::DataViewImpl::getByLabel(InputTag const& tag) const
{
  Handle<PROD> h;
  getByLabel(tag, h);
  return *h;
}

template <typename PROD>
inline PROD const*
art::DataViewImpl::getPointerByLabel(InputTag const& tag) const
{
  Handle<PROD> h;
  getByLabel(tag, h);
  return &(*h);
}

template <typename PROD>
inline art::ValidHandle<PROD>
art::DataViewImpl::getValidHandle(InputTag const& tag) const
{
  Handle<PROD> h;
  getByLabel(tag, h);
  return ValidHandle<PROD>(&(*h), *h.provenance());
}

template <typename PROD>
inline art::ValidHandle<PROD>
art::DataViewImpl::getValidHandle(ProductToken<PROD> const& token) const
{
  return getValidHandle<PROD>(token.inputTag_);
}

template <typename PROD>
inline void
art::DataViewImpl::getMany(SelectorBase const& sel,
                           std::vector<Handle<PROD>>& results) const
{
  auto const wrapped = WrappedTypeID::make<PROD>();
  ConsumesInfo::instance()->validateConsumedProduct(
    branchType_,
    md_,
    ProductInfo{ProductInfo::ConsumableType::Many, wrapped.product_type});
  std::vector<Handle<PROD>> products;
  for (auto const& qr : getMany_(wrapped, sel)) {
    Handle<PROD> result;
    convert_handle(qr, result);
    products.push_back(result);
  }
  results.swap(products);
  if (recordParents_) {
    for (auto const& h : results) {
      recordAsParent(*h.provenance());
    }
  }
}

template <typename PROD>
inline void
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
  ConsumesInfo::instance()->validateConsumedProduct(branchType_, md_, pinfo);
  auto bhv =
    getMatchingSequenceByLabel_(moduleLabel, productInstanceName, processName);
  removeNonViewableMatches_(typeID, bhv);
  ensureUniqueProduct_(
    bhv.size(), typeID, moduleLabel, productInstanceName, processName);
  return bhv;
} // getView_<>()

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
} // getView<>()

template <typename ELEMENT>
std::size_t
art::DataViewImpl::getView(InputTag const& tag,
                           std::vector<ELEMENT const*>& result) const
{
  auto bhv = getView_<ELEMENT>(tag.label(), tag.instance(), tag.process());
  std::size_t const orig_size = result.size();
  fillView_(bhv[0], result);
  return result.size() - orig_size;
} // getView<>()

template <typename ELEMENT>
bool
art::DataViewImpl::getView(std::string const& moduleLabel,
                           std::string const& productInstanceName,
                           View<ELEMENT>& result) const
{
  auto bhv = getView_<ELEMENT>(moduleLabel, productInstanceName, {});
  fillView_(bhv[0], result.vals());
  result.set_innards(bhv[0].result()->productID(),
                     bhv[0].result()->uniqueProduct());
  return true;
}

template <typename ELEMENT>
bool
art::DataViewImpl::getView(InputTag const& tag, View<ELEMENT>& result) const
{
  auto bhv = getView_<ELEMENT>(tag.label(), tag.instance(), tag.process());
  fillView_(bhv[0], result.vals());
  result.set_innards(bhv[0].result()->productID(),
                     bhv[0].result()->uniqueProduct());
  return true;
}

template <typename ELEMENT>
bool
art::DataViewImpl::getView(ViewToken<ELEMENT> const& token,
                           View<ELEMENT>& result) const
{
  return getView(token.inputTag_, result);
}

template <typename ELEMENT>
void
art::DataViewImpl::fillView_(GroupQueryResult& bh,
                             std::vector<ELEMENT const*>& result) const
{
  std::vector<void const*> erased_ptrs;
  auto product = bh.result()->uniqueProduct();

  // The lookups and the checking done in getView_ ensure that the
  // retrieved product supports the requested view.
  product->fillView(erased_ptrs);
  recordAsParent(Provenance{bh.result()});

  std::vector<ELEMENT const*> vals;
  cet::transform_all(erased_ptrs, std::back_inserter(vals), [](auto p) {
    return static_cast<ELEMENT const*>(p);
  });
  result.swap(vals);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::getProductID() const
{
  TypeID const type{typeid(PROD)};
  return getProductID_(type, "");
}

template <typename PROD>
art::ProductID
art::DataViewImpl::getProductID(std::string const& instance_name) const
{
  TypeID const type{typeid(PROD)};
  return getProductID_(type, instance_name);
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

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp)
{
  return put<PROD>(std::move(edp), "");
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       FullSemantic<Level::Run> const semantic)
{
  return put<PROD>(std::move(edp), "", semantic);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       FragmentSemantic<Level::Run> const semantic)
{
  return put<PROD>(std::move(edp), "", semantic);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       RangedFragmentSemantic<Level::Run> semantic)
{
  return put<PROD>(std::move(edp), "", std::move(semantic));
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       std::string const& instance,
                       FullSemantic<Level::Run>)
{
  return put<PROD>(std::move(edp), instance, RangeSet::forRun(runID()));
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       std::string const& instance,
                       FragmentSemantic<Level::Run>)
{
  static_assert(
    detail::CanBeAggregated<PROD>::value,
    "\n\n"
    "art error: A Run product put with the semantic 'RunFragment'\n"
    "           must be able to be aggregated. Please add the appropriate\n"
    "              void aggregate(T const&)\n"
    "           function to your class, or contact artists@fnal.gov.\n");
  if (rangeSet_.collapse().is_full_run()) {
    throw Exception{errors::ProductPutFailure, "Run::put"}
      << "\nCannot put a product corresponding to a full Run using\n"
      << "art::runFragment().  This can happen if you attempted to\n"
      << "put a product at beginRun using art::runFragment().\n"
      << "Please use either:\n"
      << "   art::fullRun(), or\n"
      << "   art::runFragment(art::RangeSet const&)\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  return put<PROD>(std::move(edp), instance, rangeSet_);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       std::string const& instance,
                       RangedFragmentSemantic<Level::Run> semantic)
{
  static_assert(
    detail::CanBeAggregated<PROD>::value,
    "\n\n"
    "art error: A Run product put with the semantic 'RunFragment'\n"
    "           must be able to be aggregated. Please add the appropriate\n"
    "              void aggregate(T const&)\n"
    "           function to your class, or contact artists@fnal.gov.\n");
  if (semantic.rs.collapse().is_full_run()) {
    throw Exception{errors::ProductPutFailure, "Run::put"}
      << "\nCannot put a product corresponding to a full Run using\n"
      << "art::runFragment(art::RangeSet&).  Please use:\n"
      << "   art::fullRun()\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  return put<PROD>(std::move(edp), instance, semantic.rs);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       FullSemantic<Level::SubRun> const semantic)
{
  return put<PROD>(std::move(edp), "", semantic);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       FragmentSemantic<Level::SubRun> const semantic)
{
  return put<PROD>(std::move(edp), "", semantic);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       RangedFragmentSemantic<Level::SubRun> semantic)
{
  return put<PROD>(std::move(edp), "", std::move(semantic));
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       std::string const& instance,
                       FullSemantic<Level::SubRun>)
{
  return put<PROD>(std::move(edp), instance, RangeSet::forSubRun(subRunID()));
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       std::string const& instance,
                       FragmentSemantic<Level::SubRun>)
{
  static_assert(
    detail::CanBeAggregated<PROD>::value,
    "\n\n"
    "art error: A SubRun product put with the semantic 'SubRunFragment'\n"
    "           must be able to be aggregated. Please add the appropriate\n"
    "              void aggregate(T const&)\n"
    "           function to your class, or contact artists@fnal.gov.\n");
  if (rangeSet_.collapse().is_full_subRun()) {
    throw Exception(errors::ProductPutFailure, "SubRun::put")
      << "\nCannot put a product corresponding to a full SubRun using\n"
      << "art::subRunFragment().  This can happen if you attempted to\n"
      << "put a product at beginSubRun using art::subRunFragment().\n"
      << "Please use either:\n"
      << "   art::fullSubRun(), or\n"
      << "   art::subRunFragment(art::RangeSet const&)\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  return put<PROD>(std::move(edp), instance, rangeSet_);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       std::string const& instance,
                       RangedFragmentSemantic<Level::SubRun> semantic)
{
  static_assert(
    detail::CanBeAggregated<PROD>::value,
    "\n\n"
    "art error: A SubRun product put with the semantic 'SubRunFragment'\n"
    "           must be able to be aggregated. Please add the appropriate\n"
    "              void aggregate(T const&)\n"
    "           function to your class, or contact artists@fnal.gov.\n");
  if (semantic.rs.collapse().is_full_subRun()) {
    throw Exception{errors::ProductPutFailure, "Run::put"}
      << "\nCannot put a product corresponding to a full SubRun using\n"
      << "art::subRunFragment(art::RangeSet&).  Please use:\n"
      << "   art::fullSubRun()\n"
      << "or contact artists@fnal.gov for assistance.\n";
  }
  return put<PROD>(std::move(edp), instance, semantic.rs);
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp, std::string const& instance)
{
  TypeID const tid{typeid(PROD)};
  if (edp.get() == nullptr) {
    throw Exception(errors::NullPointerError)
      << "Event::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << tid << ".\n"
      << "The specified productInstanceName was '" << instance << "'.\n";
  }
  std::unique_ptr<TypeLabel> typeLabel{nullptr};
  if (md_.isEmulatedModule()) {
    typeLabel = std::make_unique<TypeLabel>(
      tid, instance, SupportsView<PROD>::value, md_.moduleLabel());
  } else {
    typeLabel = std::make_unique<TypeLabel>(
      tid, instance, SupportsView<PROD>::value, false /*not used*/);
  }
  auto const& bd = getProductDescription_(*typeLabel);
  assert(bd.productID() != ProductID::invalid());
  auto wp = std::make_unique<Wrapper<PROD>>(std::move(edp));
  bool result = false;
  if ((branchType_ == InRun) || (branchType_ == InSubRun)) {
    rangeSet_.collapse();
    result =
      putProducts_.emplace(*typeLabel, PMValue{std::move(wp), bd, rangeSet_})
        .second;
  } else {
    result =
      putProducts_
        .emplace(*typeLabel, PMValue{std::move(wp), bd, RangeSet::invalid()})
        .second;
  }
  if (!result) {
    cet::HorizontalRule rule{30};
    throw Exception(errors::ProductPutFailure)
      << "Event::put: Attempt to put multiple products with the\n"
      << "            following description onto the Event.\n"
      << "            Products must be unique per Event.\n"
      << rule('=') << '\n'
      << bd << rule('=') << '\n';
  }
  return bd.productID();
}

template <typename PROD>
art::ProductID
art::DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                       std::string const& instance,
                       RangeSet const& rs)
{
  TypeID const tid{typeid(PROD)};
  if (edp.get() == nullptr) {
    throw Exception(errors::NullPointerError)
      << "Event::put: A null unique_ptr was passed to 'put'.\n"
      << "The pointer is of type " << tid << ".\n"
      << "The specified productInstanceName was '" << instance << "'.\n";
  }
  if (!rs.is_valid()) {
    throw Exception{errors::ProductPutFailure, "SubRun::put"}
      << "\nCannot put a product with an invalid RangeSet.\n"
      << "Please contact artists@fnal.gov.\n";
  }

  std::unique_ptr<TypeLabel> typeLabel{nullptr};
  if (md_.isEmulatedModule()) {
    typeLabel = std::make_unique<TypeLabel>(
      tid, instance, SupportsView<PROD>::value, md_.moduleLabel());
  } else {
    typeLabel = std::make_unique<TypeLabel>(
      tid, instance, SupportsView<PROD>::value, false /*not used*/);
  }
  auto const& bd = getProductDescription_(*typeLabel);
  auto wp = std::make_unique<Wrapper<PROD>>(std::move(edp));
  auto result =
    putProducts_.emplace(*typeLabel, PMValue{std::move(wp), bd, rs});
  if (!result.second) {
    cet::HorizontalRule rule{30};
    throw Exception(errors::ProductPutFailure)
      << "Event::put: Attempt to put multiple products with the\n"
      << "            following description onto the Event.\n"
      << "            Products must be unique per Event.\n"
      << rule('=') << '\n'
      << bd << rule('=') << '\n';
  }
  return bd.productID();
}

#endif /* art_Framework_Principal_DataViewImpl_h */

// Local Variables:
// mode: c++
// End:
