#ifndef art_Framework_Principal_Event_h
#define art_Framework_Principal_Event_h
// vim: set sw=2 expandtab :

// ===============================================================
// This is the primary interface for accessing products from a
// triggered event and inserting new derived products.
//
// For its usage, see "art/Framework/Principal/ProductRetriever.h"
// ===============================================================

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/ProductInserter.h"
#include "art/Framework/Principal/ProductRetriever.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/fwd.h"
#include "canvas/Persistency/Provenance/EventID.h"

#include <memory>
#include <optional>

namespace art {

  class Event final : private ProductRetriever {
  public:
    template <typename T>
    using HandleT = Handle<T>;

    ~Event();

    explicit Event(EventPrincipal const& ep,
                   ModuleContext const& mc,
                   std::optional<ProductInserter> inserter = std::nullopt);

    Event(Event const&) = delete;
    Event(Event&&) = delete;
    Event& operator=(Event const&) = delete;
    Event& operator=(Event&&) = delete;

    EventID id() const;
    RunNumber_t run() const;
    SubRunNumber_t subRun() const;
    EventNumber_t event() const;
    Timestamp time() const;
    bool isRealData() const;
    EventAuxiliary::ExperimentType experimentType() const;

    SubRun const& getSubRun() const;
    Run const& getRun() const;

    ProcessHistory const& processHistory() const;
    ProcessHistoryID const& processHistoryID() const;

    using ProductRetriever::getHandle;
    using ProductRetriever::getInputTags;
    using ProductRetriever::getMany;
    using ProductRetriever::getProduct;
    using ProductRetriever::getProductTokens;
    using ProductRetriever::getValidHandle;
    using ProductRetriever::getView;

    using ProductRetriever::getProductDescription;
    using ProductRetriever::getProductID;

    using ProductRetriever::getProcessParameterSet;
    using ProductRetriever::productGetter;

    // Obsolete interface (will be deprecated)
    using ProductRetriever::get;
    using ProductRetriever::getByLabel;

    template <typename PROD>
    PutHandle<PROD>
    put(std::unique_ptr<PROD>&& edp, std::string const& instance = {})
    {
      assert(inserter_);
      return inserter_->put(move(edp), instance);
    }

  private:
    void commitProducts();
    void commitProducts(
      bool const checkProducts,
      std::map<TypeLabel, BranchDescription> const* expectedProducts);

    // Give access to commitProducts(...).
    friend class detail::Analyzer;
    friend class detail::Filter;
    friend class detail::Producer;
    friend class ProducingService;

    std::optional<ProductInserter> inserter_;
    EventPrincipal const& eventPrincipal_;
    SubRun const subRun_;
  };

} // namespace art

#endif /* art_Framework_Principal_Event_h */

// Local Variables:
// mode: c++
// End:
