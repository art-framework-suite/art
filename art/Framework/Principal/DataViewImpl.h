#ifndef art_Framework_Principal_DataViewImpl_h
#define art_Framework_Principal_DataViewImpl_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ConsumesInfo.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/ProcessTag.h"
#include "art/Framework/Principal/RangeSetsSupported.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Selector.h"
#include "art/Framework/Principal/detail/type_label_for.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "art/Persistency/Common/fwd.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/ProductSemantics.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Persistency/Common/WrappedTypeID.h"
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
#include "canvas/Persistency/Provenance/canonicalProductName.h"
#include "canvas/Persistency/Provenance/type_aliases.h"
#include "canvas/Utilities/InputTag.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib_except/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/tsan.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <map>
#include <memory>
#include <mutex>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace art {

  class EDProductGetter;
  class ResultsProducer;

  namespace detail {
    class Analyzer;
    class Filter;
    class Producer;
  }

  class DataViewImpl {

    // Give access to movePutProductsToPrincipal(...).
    friend class detail::Analyzer;
    friend class detail::Filter;
    friend class detail::Producer;
    friend class ResultsProducer;
    friend class ProducingService;

  public:
    ~DataViewImpl();
    explicit DataViewImpl(BranchType bt,
                          Principal const& p,
                          ModuleContext const& mc,
                          bool recordParents,
                          RangeSet const& rs = RangeSet::invalid());
    DataViewImpl(DataViewImpl const&) = delete;
    DataViewImpl(DataViewImpl&&) = delete;
    DataViewImpl& operator=(DataViewImpl const&) = delete;
    DataViewImpl& operator=(DataViewImpl&) = delete;

    // Miscellaneous functionality
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
    ProcessHistory const& processHistory() const;
    template <typename PROD>
    bool removeCachedProduct(Handle<PROD>&) const;
    EDProductGetter const* productGetter(ProductID const pid) const;
    bool getProcessParameterSet(std::string const& process,
                                fhicl::ParameterSet&) const;

    // Product retrieval
    template <typename PROD>
    PROD const& getProduct(InputTag const& tag) const;
    template <typename PROD>
    PROD const& getProduct(ProductToken<PROD> const& token) const;

    // Product retrieval with provenance access
    template <typename PROD>
    Handle<PROD> getHandle(SelectorBase const&) const;
    template <typename PROD>
    Handle<PROD> getHandle(ProductID const pid) const;
    template <typename PROD>
    Handle<PROD> getHandle(InputTag const& tag) const;
    template <typename PROD>
    Handle<PROD> getHandle(ProductToken<PROD> const& token) const;

    // Product retrieval with provenance access (guaranteed valid handle)
    template <typename PROD>
    ValidHandle<PROD> getValidHandle(InputTag const& tag) const;
    template <typename PROD>
    ValidHandle<PROD> getValidHandle(ProductToken<PROD> const& token) const;

    // Multiple product retrievals
    template <typename PROD>
    std::vector<InputTag> getInputTags(
      SelectorBase const& selector = MatchAllSelector{}) const;
    template <typename PROD>
    std::vector<ProductToken<PROD>> getProductTokens(
      SelectorBase const& selector = MatchAllSelector{}) const;
    template <typename PROD>
    std::vector<Handle<PROD>> getMany(
      SelectorBase const& selector = MatchAllSelector{}) const;

    // Obsolete product-retrieval (will be deprecated)
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
    template <typename PROD>
    bool getByLabel(InputTag const& tag, Handle<PROD>& result) const;

    // Deprecated
    template <typename PROD>
    [[deprecated(
      "\n\nart warning: Please use getHandle<T>(token) instead.\n\n")]] bool
    getByToken(ProductToken<PROD> const& token, Handle<PROD>& result) const;
    template <typename PROD>
    [[deprecated("\n\nart warning: Please use getHandle<T>(tag).product() "
                 "instead.\n\n")]] PROD const*
    getPointerByLabel(InputTag const& tag) const;
    template <typename PROD>
    [[deprecated(
      "\n\nart warning: Please use getMany<T>(selector) instead.\n\n")]] void
    getMany(SelectorBase const&, std::vector<Handle<PROD>>& results) const;
    template <typename PROD>
    [[deprecated("\n\nart warning: Please use getMany<T>() instead.\n\n")]] void
    getManyByType(std::vector<Handle<PROD>>& results) const;

    // View retrieval
    template <typename ELEMENT>
    std::size_t getView(std::string const& moduleLabel,
                        std::string const& productInstanceName,
                        std::string const& processName,
                        std::vector<ELEMENT const*>& result) const;
    template <typename ELEMENT>
    std::size_t getView(std::string const& moduleLabel,
                        std::string const& productInstanceName,
                        std::vector<ELEMENT const*>& result) const;
    template <typename ELEMENT>
    std::size_t getView(InputTag const&,
                        std::vector<ELEMENT const*>& result) const;
    template <typename ELEMENT>
    std::size_t getView(ViewToken<ELEMENT> const&,
                        std::vector<ELEMENT const*>& result) const;
    template <typename ELEMENT>
    bool getView(std::string const& moduleLabel,
                 std::string const& productInstanceName,
                 std::string const& processName,
                 View<ELEMENT>& result) const;
    template <typename ELEMENT>
    bool getView(std::string const& moduleLabel,
                 std::string const& productInstanceName,
                 View<ELEMENT>& result) const;
    template <typename ELEMENT>
    bool getView(InputTag const&, View<ELEMENT>& result) const;
    template <typename ELEMENT>
    bool getView(ViewToken<ELEMENT> const&, View<ELEMENT>& result) const;

    // Product ID and description
    template <typename T>
    ProductID getProductID(std::string const& instance_name = "") const;

    cet::exempt_ptr<BranchDescription const> getProductDescription(
      ProductID) const;

    // Product insertion - all processing levels
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance = {});

    // Product insertion -- run/subrun
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  std::string const& instance,
                  RangeSet const& rs);

    // Product insertion - run
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FullSemantic<Level::Run> semantic);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FragmentSemantic<Level::Run> semantic);
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

    // Product insertion - subrun
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FullSemantic<Level::SubRun> semantic);
    template <typename PROD>
    ProductID put(std::unique_ptr<PROD>&& edp,
                  FragmentSemantic<Level::SubRun> semantic);
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

    void movePutProductsToPrincipal(Principal& principal);
    void movePutProductsToPrincipal(
      Principal& principal,
      bool const checkProducts,
      std::map<TypeLabel, BranchDescription> const* expectedProducts);

  private:
    struct PMValue {
      PMValue(std::unique_ptr<EDProduct>&& p,
              BranchDescription const& b,
              RangeSet const& r)
        : prod_{std::move(p)}, bd_{b}, rs_{r}
      {}
      std::unique_ptr<EDProduct> prod_;
      BranchDescription const& bd_;
      RangeSet rs_;
    };

    std::string const& getProcessName_(std::string const&) const;
    BranchDescription const& getProductDescription_(
      TypeID const& type,
      std::string const& instance,
      bool const alwaysEnableLookupOfProducedProducts = false) const;
    void recordAsParent_(cet::exempt_ptr<Group const> grp) const;
    cet::exempt_ptr<Group const> getContainerForView_(
      TypeID const&,
      std::string const& moduleLabel,
      std::string const& productInstanceName,
      ProcessTag const& processTag) const;

    // MEMBER DATA
  private:
    // Protects use of retrievedProducts_,
    // putProducts_, and rangeSet_.
    mutable std::recursive_mutex mutex_{};

    // Is this an Event, a Run, a SubRun, or a Results.
    BranchType const branchType_;

    // The principal we are operating on.
    Principal const& principal_;

    // The context of the currently processing module.
    ModuleContext const& mc_;

    // The module we were created for.
    ModuleDescription const& md_;

    // If we are constructed as a non-const Event, then we
    // can be used to put products into the Principal, so
    // we need to record retrieved products into
    // retrievedProducts_ to track parentage of any
    // products we put.
    bool const recordParents_;

    // The RangeSet to be used by any products put by the user.
    // Cannot be const because we call collapse() on it.
    RangeSet rangeSet_{RangeSet::invalid()};

    // The set of products retrieved from the principal.
    // We use this to track parentage of any products
    // we put.
    mutable std::set<ProductID> retrievedProducts_{};

    // The set of products which have been put by the user.
    // We own them.
    std::map<TypeLabel, PMValue> putProducts_{};
  };

  template <typename PROD>
  ProductID
  DataViewImpl::getProductID(std::string const& instance /* = "" */) const
  {
    std::lock_guard lock{mutex_};
    TypeID const type{typeid(PROD)};
    auto const& product_name = canonicalProductName(
      type.friendlyClassName(), md_.moduleLabel(), instance, md_.processName());
    ProductID const pid{product_name};
    auto desc = principal_.getProductDescription(pid);
    if (!desc) {
      throw art::Exception(art::errors::ProductRegistrationFailure,
                           "DataViewImpl::getProductID: error while trying to "
                           "retrieve product description:\n")
        << "No product is registered for\n"
        << "  process name:                '" << md_.processName() << "'\n"
        << "  module label:                '" << md_.moduleLabel() << "'\n"
        << "  product friendly class name: '" << type.friendlyClassName()
        << "'\n"
        << "  product instance name:       '" << instance << "'\n"
        << "  branch type:                 '" << branchType_ << "'\n";
    }
    // The description object is owned by either the source or the
    // event processor, whose lifetimes exceed that of the
    // DataViewImpl object.  It is therefore safe to dereference.
    return desc->productID();
  }

  // =========================================================================
  template <typename PROD>
  PROD const&
  DataViewImpl::getProduct(InputTag const& tag) const
  {
    return *getValidHandle<PROD>(tag);
  }

  template <typename PROD>
  PROD const&
  DataViewImpl::getProduct(ProductToken<PROD> const& token) const
  {
    return *getValidHandle(token);
  }

  // =========================================================================
  template <typename PROD>
  Handle<PROD>
  DataViewImpl::getHandle(SelectorBase const& sel) const
  {
    std::lock_guard lock{mutex_};
    // We do *not* track whether consumes was called for a SelectorBase.
    ProcessTag const processTag{"", md_.processName()};
    auto qr = principal_.getBySelector(
      mc_, WrappedTypeID::make<PROD>(), sel, processTag);
    bool const ok = qr.succeeded() && !qr.failed();
    if (recordParents_ && ok) {
      recordAsParent_(qr.result());
    }
    return Handle<PROD>{qr};
  }

  template <typename PROD>
  Handle<PROD>
  DataViewImpl::getHandle(ProductID const pid) const
  {
    std::lock_guard lock{mutex_};
    auto qr = principal_.getByProductID(pid);
    bool const ok = qr.succeeded() && !qr.failed();
    if (recordParents_ && ok) {
      recordAsParent_(qr.result());
    }
    return Handle<PROD>{qr};
  }

  template <typename PROD>
  Handle<PROD>
  DataViewImpl::getHandle(InputTag const& tag) const
  {
    std::lock_guard lock{mutex_};
    auto const wrapped = WrappedTypeID::make<PROD>();
    ProcessTag const processTag{tag.process(), md_.processName()};
    ProductInfo const pinfo{ProductInfo::ConsumableType::Product,
                            wrapped.product_type,
                            tag.label(),
                            tag.instance(),
                            processTag};
    ConsumesInfo::instance()->validateConsumedProduct(branchType_, md_, pinfo);
    GroupQueryResult qr = principal_.getByLabel(
      mc_, wrapped, tag.label(), tag.instance(), processTag);
    bool const ok = qr.succeeded() && !qr.failed();
    if (recordParents_ && ok) {
      recordAsParent_(qr.result());
    }
    return Handle<PROD>{qr};
  }

  template <typename PROD>
  Handle<PROD>
  DataViewImpl::getHandle(ProductToken<PROD> const& token) const
  {
    return getHandle<PROD>(token.inputTag_);
  }

  // =========================================================================
  template <typename PROD>
  ValidHandle<PROD>
  DataViewImpl::getValidHandle(InputTag const& tag) const
  {
    Handle<PROD> h;
    getByLabel(tag, h);
    return ValidHandle{h.product(), *h.provenance()};
  }

  template <typename PROD>
  ValidHandle<PROD>
  DataViewImpl::getValidHandle(ProductToken<PROD> const& token) const
  {
    return getValidHandle<PROD>(token.inputTag_);
  }

  template <typename PROD>
  std::vector<InputTag>
  DataViewImpl::getInputTags(SelectorBase const& selector) const
  {
    auto const wrapped = WrappedTypeID::make<PROD>();
    ProcessTag const processTag{"", md_.processName()};
    return principal_.getInputTags(mc_, wrapped, selector, processTag);
  }

  template <typename PROD>
  std::vector<ProductToken<PROD>>
  DataViewImpl::getProductTokens(SelectorBase const& selector) const
  {
    auto const tags = getInputTags<PROD>(selector);
    std::vector<ProductToken<PROD>> tokens;
    tokens.reserve(tags.size());
    cet::transform_all(tags, back_inserter(tokens), [](auto const& tag) {
      return ProductToken<PROD>{tag};
    });
    return tokens;
  }

  template <typename PROD>
  std::vector<Handle<PROD>>
  DataViewImpl::getMany(SelectorBase const& sel) const
  {
    std::lock_guard lock{mutex_};
    auto const wrapped = WrappedTypeID::make<PROD>();
    ConsumesInfo::instance()->validateConsumedProduct(
      branchType_,
      md_,
      ProductInfo{ProductInfo::ConsumableType::Many, wrapped.product_type});
    ProcessTag const processTag{"", md_.processName()};
    std::vector<Handle<PROD>> products;
    for (auto const& qr : principal_.getMany(mc_, wrapped, sel, processTag)) {
      products.emplace_back(qr);
      if (recordParents_) {
        recordAsParent_(qr.result());
      }
    }
    return products;
  }

  template <typename ELEMENT>
  std::size_t
  DataViewImpl::getView(std::string const& moduleLabel,
                        std::string const& productInstanceName,
                        std::string const& processName,
                        std::vector<ELEMENT const*>& result) const
  {
    std::lock_guard lock{mutex_};
    std::size_t const orig_size = result.size();
    auto grp = getContainerForView_(TypeID{typeid(ELEMENT)},
                                    moduleLabel,
                                    productInstanceName,
                                    ProcessTag{processName, md_.processName()});
    if (recordParents_) {
      recordAsParent_(grp);
    }
    auto const view = grp->uniqueProduct()->getView();
    std::vector<ELEMENT const*> castedView;
    for (auto p : view) {
      castedView.push_back(static_cast<ELEMENT const*>(p));
    }
    result = std::move(castedView);
    return result.size() - orig_size;
  }

  template <typename ELEMENT>
  std::size_t
  DataViewImpl::getView(std::string const& moduleLabel,
                        std::string const& productInstanceName,
                        std::vector<ELEMENT const*>& result) const
  {
    return getView(moduleLabel, productInstanceName, {}, result);
  }

  template <typename ELEMENT>
  std::size_t
  DataViewImpl::getView(InputTag const& tag,
                        std::vector<ELEMENT const*>& result) const
  {
    return getView(tag.label(), tag.instance(), tag.process(), result);
  }

  template <typename ELEMENT>
  std::size_t
  DataViewImpl::getView(ViewToken<ELEMENT> const& token,
                        std::vector<ELEMENT const*>& result) const
  {
    return getView(token.inputTag_.label(),
                   token.inputTag_.instance(),
                   token.inputTag_.process(),
                   result);
  }

  template <typename ELEMENT>
  bool
  DataViewImpl::getView(std::string const& moduleLabel,
                        std::string const& productInstanceName,
                        std::string const& processName,
                        View<ELEMENT>& result) const
  {
    std::lock_guard lock{mutex_};
    auto grp = getContainerForView_(TypeID{typeid(ELEMENT)},
                                    moduleLabel,
                                    productInstanceName,
                                    ProcessTag{processName, md_.processName()});
    if (recordParents_) {
      recordAsParent_(grp);
    }
    auto const view = grp->uniqueProduct()->getView();
    std::vector<ELEMENT const*> castedView;
    for (auto p : view) {
      castedView.push_back(static_cast<ELEMENT const*>(p));
    }
    result = View{move(castedView), grp->productID(), grp->uniqueProduct()};
    return true;
  }

  template <typename ELEMENT>
  bool
  DataViewImpl::getView(std::string const& moduleLabel,
                        std::string const& productInstanceName,
                        View<ELEMENT>& result) const
  {
    return getView(moduleLabel, productInstanceName, {}, result);
  }

  template <typename ELEMENT>
  bool
  DataViewImpl::getView(InputTag const& tag, View<ELEMENT>& result) const
  {
    return getView(tag.label(), tag.instance(), tag.process(), result);
  }

  template <typename ELEMENT>
  bool
  DataViewImpl::getView(ViewToken<ELEMENT> const& token,
                        View<ELEMENT>& result) const
  {
    return getView(token.inputTag_.label(),
                   token.inputTag_.instance(),
                   token.inputTag_.process(),
                   result);
  }

  // =======================================================================
  // Obsolete (will be deprecated)
  template <typename PROD>
  bool
  DataViewImpl::get(SelectorBase const& sel, Handle<PROD>& result) const
  {
    result = getHandle<PROD>(sel);
    return static_cast<bool>(result);
  }

  template <typename PROD>
  bool
  DataViewImpl::get(ProductID const pid, Handle<PROD>& result) const
  {
    result = getHandle<PROD>(pid);
    return static_cast<bool>(result);
  }

  template <typename PROD>
  bool
  DataViewImpl::getByLabel(std::string const& moduleLabel,
                           std::string const& productInstanceName,
                           std::string const& processName,
                           Handle<PROD>& result) const
  {
    result = getHandle<PROD>({moduleLabel, productInstanceName, processName});
    return static_cast<bool>(result);
  }

  template <typename PROD>
  bool
  DataViewImpl::getByLabel(std::string const& moduleLabel,
                           std::string const& instance,
                           Handle<PROD>& result) const
  {
    result = getHandle<PROD>({moduleLabel, instance});
    return static_cast<bool>(result);
  }

  template <typename PROD>
  bool
  DataViewImpl::getByLabel(InputTag const& tag, Handle<PROD>& result) const
  {
    result = getHandle<PROD>(tag);
    return static_cast<bool>(result);
  }

  template <typename PROD>
  PROD const*
  DataViewImpl::getPointerByLabel(InputTag const& tag) const
  {
    Handle<PROD> h;
    getByLabel(tag, h);
    return h.product();
  }

  template <typename PROD>
  bool
  DataViewImpl::getByToken(ProductToken<PROD> const& token,
                           Handle<PROD>& result) const
  {
    result = getHandle(token);
    return static_cast<bool>(result);
  }

  template <typename PROD>
  void
  DataViewImpl::getMany(SelectorBase const& sel,
                        std::vector<Handle<PROD>>& results) const
  {
    results = getMany<PROD>(sel);
  }

  template <typename PROD>
  void
  DataViewImpl::getManyByType(std::vector<Handle<PROD>>& results) const
  {
    results = getMany<PROD>();
  }

  // =======================================================================
  // Product-insertion implementation
  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp, std::string const& instance)
  {
    std::lock_guard lock{mutex_};
    TypeID const tid{typeid(PROD)};
    if (edp.get() == nullptr) {
      throw art::Exception(errors::NullPointerError)
        << "Event::put: A null unique_ptr was passed to 'put'.\n"
        << "The pointer is of type " << tid << ".\n"
        << "The specified productInstanceName was '" << instance << "'.\n";
    }
    auto const& bd = getProductDescription_(tid, instance, true);
    assert(bd.productID() != ProductID::invalid());
    auto const typeLabel =
      detail::type_label_for(tid, instance, SupportsView<PROD>::value, md_);
    auto wp = std::make_unique<Wrapper<PROD>>(move(edp));
    auto const& rs = detail::range_sets_supported(branchType_) ?
                       rangeSet_.collapse() :
                       RangeSet::invalid();
    bool const result =
      putProducts_.try_emplace(typeLabel, PMValue{std::move(wp), bd, rs})
        .second;
    if (!result) {
      cet::HorizontalRule rule{30};
      throw art::Exception(errors::ProductPutFailure)
        << "Event::put: Attempt to put multiple products with the\n"
        << "            following description onto the Event.\n"
        << "            Products must be unique per Event.\n"
        << rule('=') << '\n'
        << bd << rule('=') << '\n';
    }
    return bd.productID();
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    FullSemantic<Level::Run> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    FragmentSemantic<Level::Run> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    RangedFragmentSemantic<Level::Run> semantic)
  {
    return put(move(edp), "", std::move(semantic));
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    std::string const& instance,
                    FullSemantic<Level::Run>)
  {
    return put(move(edp), instance, RangeSet::forRun(runID()));
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    std::string const& instance,
                    FragmentSemantic<Level::Run>)
  {
    std::lock_guard lock{mutex_};
    static_assert(
      detail::CanBeAggregated<PROD>::value,
      "\n\n"
      "art error: A Run product put with the semantic 'RunFragment'\n"
      "           must be able to be aggregated. Please add the appropriate\n"
      "              void aggregate(T const&)\n"
      "           function to your class, or contact artists@fnal.gov.\n");
    if (rangeSet_.collapse().is_full_run()) {
      throw art::Exception{errors::ProductPutFailure, "Run::put"}
        << "\nCannot put a product corresponding to a full Run using\n"
        << "art::runFragment().  This can happen if you attempted to\n"
        << "put a product at beginRun using art::runFragment().\n"
        << "Please use either:\n"
        << "   art::fullRun(), or\n"
        << "   art::runFragment(art::RangeSet const&)\n"
        << "or contact artists@fnal.gov for assistance.\n";
    }
    return put(move(edp), instance, rangeSet_);
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    std::string const& instance,
                    RangedFragmentSemantic<Level::Run> semantic)
  {
    std::lock_guard lock{mutex_};
    static_assert(
      detail::CanBeAggregated<PROD>::value,
      "\n\n"
      "art error: A Run product put with the semantic 'RunFragment'\n"
      "           must be able to be aggregated. Please add the appropriate\n"
      "              void aggregate(T const&)\n"
      "           function to your class, or contact artists@fnal.gov.\n");
    if (semantic.rs.collapse().is_full_run()) {
      throw art::Exception{errors::ProductPutFailure, "Run::put"}
        << "\nCannot put a product corresponding to a full Run using\n"
        << "art::runFragment(art::RangeSet&).  Please use:\n"
        << "   art::fullRun()\n"
        << "or contact artists@fnal.gov for assistance.\n";
    }
    return put(move(edp), instance, semantic.rs);
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    FullSemantic<Level::SubRun> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    FragmentSemantic<Level::SubRun> const semantic)
  {
    return put(move(edp), "", semantic);
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    RangedFragmentSemantic<Level::SubRun> semantic)
  {
    return put(move(edp), "", std::move(semantic));
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    std::string const& instance,
                    FullSemantic<Level::SubRun>)
  {
    return put(move(edp), instance, RangeSet::forSubRun(subRunID()));
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    std::string const& instance,
                    FragmentSemantic<Level::SubRun>)
  {
    std::lock_guard lock{mutex_};
    static_assert(
      detail::CanBeAggregated<PROD>::value,
      "\n\n"
      "art error: A SubRun product put with the semantic 'SubRunFragment'\n"
      "           must be able to be aggregated. Please add the appropriate\n"
      "              void aggregate(T const&)\n"
      "           function to your class, or contact artists@fnal.gov.\n");
    if (rangeSet_.collapse().is_full_subRun()) {
      throw art::Exception(errors::ProductPutFailure, "SubRun::put")
        << "\nCannot put a product corresponding to a full SubRun using\n"
        << "art::subRunFragment().  This can happen if you attempted to\n"
        << "put a product at beginSubRun using art::subRunFragment().\n"
        << "Please use either:\n"
        << "   art::fullSubRun(), or\n"
        << "   art::subRunFragment(art::RangeSet const&)\n"
        << "or contact artists@fnal.gov for assistance.\n";
    }
    return put(move(edp), instance, rangeSet_);
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    std::string const& instance,
                    RangedFragmentSemantic<Level::SubRun> semantic)
  {
    std::lock_guard lock{mutex_};
    static_assert(
      detail::CanBeAggregated<PROD>::value,
      "\n\n"
      "art error: A SubRun product put with the semantic 'SubRunFragment'\n"
      "           must be able to be aggregated. Please add the appropriate\n"
      "              void aggregate(T const&)\n"
      "           function to your class, or contact artists@fnal.gov.\n");
    if (semantic.rs.collapse().is_full_subRun()) {
      throw art::Exception{errors::ProductPutFailure, "Run::put"}
        << "\nCannot put a product corresponding to a full SubRun using\n"
        << "art::subRunFragment(art::RangeSet&).  Please use:\n"
        << "   art::fullSubRun()\n"
        << "or contact artists@fnal.gov for assistance.\n";
    }
    return put(move(edp), instance, semantic.rs);
  }

  template <typename PROD>
  ProductID
  DataViewImpl::put(std::unique_ptr<PROD>&& edp,
                    std::string const& instance,
                    RangeSet const& rs)
  {
    std::lock_guard lock{mutex_};
    TypeID const tid{typeid(PROD)};
    if (edp.get() == nullptr) {
      throw art::Exception(errors::NullPointerError)
        << "Event::put: A null unique_ptr was passed to 'put'.\n"
        << "The pointer is of type " << tid << ".\n"
        << "The specified productInstanceName was '" << instance << "'.\n";
    }
    if (!rs.is_valid()) {
      throw art::Exception{errors::ProductPutFailure, "SubRun::put"}
        << "\nCannot put a product with an invalid RangeSet.\n"
        << "Please contact artists@fnal.gov.\n";
    }
    auto const& bd = getProductDescription_(tid, instance, true);
    assert(bd.productID() != ProductID::invalid());
    auto const typeLabel =
      detail::type_label_for(tid, instance, SupportsView<PROD>::value, md_);
    auto wp = std::make_unique<Wrapper<PROD>>(move(edp));
    auto result =
      putProducts_.try_emplace(typeLabel, PMValue{move(wp), bd, rs}).second;
    if (!result) {
      constexpr cet::HorizontalRule rule{30};
      throw art::Exception(errors::ProductPutFailure)
        << "Event::put: Attempt to put multiple products with the\n"
        << "            following description onto the Event.\n"
        << "            Products must be unique per Event.\n"
        << rule('=') << '\n'
        << bd << rule('=') << '\n';
    }
    return bd.productID();
  }

  template <typename PROD>
  bool
  DataViewImpl::removeCachedProduct(Handle<PROD>& h) const
  {
    std::lock_guard lock{mutex_};
    bool result{false};
    if (h.isValid() && !h.provenance()->produced()) {
      principal_.removeCachedProduct(h.id());
      h.clear();
      result = true;
    }
    return result;
  }

  template <typename PROD>
  std::ostream&
  operator<<(std::ostream& os, Handle<PROD> const& h)
  {
    os << h.product() << " " << h.provenance() << " " << h.id();
    return os;
  }

} // namespace art

#endif /* art_Framework_Principal_DataViewImpl_h */

// Local Variables:
// mode: c++
// End:
