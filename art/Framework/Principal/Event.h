#ifndef art_Framework_Principal_Event_h
#define art_Framework_Principal_Event_h
// vim: set sw=2 expandtab :

// =============================================================
// This is the primary interface for accessing products from a
// triggered event and inserting new derived products.
//
// For its usage, see "art/Framework/Principal/DataViewImpl.h"
// =============================================================

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/fwd.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/History.h"

#include <memory>

namespace art {

  class Event final : private DataViewImpl {
  public:
    template <typename T>
    using HandleT = Handle<T>;

    ~Event();

    explicit Event(EventPrincipal const& ep,
                   ModuleContext const& mc,
                   bool recordParents = true);

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

    History const& history() const;
    ProcessHistory const& processHistory() const;
    ProcessHistoryID const& processHistoryID() const;

    using DataViewImpl::put;

    using DataViewImpl::getHandle;
    using DataViewImpl::getInputTags;
    using DataViewImpl::getMany;
    using DataViewImpl::getProduct;
    using DataViewImpl::getProductTokens;
    using DataViewImpl::getValidHandle;
    using DataViewImpl::getView;

    using DataViewImpl::getProductDescription;
    using DataViewImpl::getProductID;

    using DataViewImpl::getProcessParameterSet;
    using DataViewImpl::productGetter;

    // Obsolete interface (will be deprecated)
    using DataViewImpl::get;
    using DataViewImpl::getByLabel;

    using DataViewImpl::movePutProductsToPrincipal;

  private:
    EventPrincipal const& eventPrincipal_;
    std::unique_ptr<SubRun const> const subRun_;
  };

} // namespace art

#endif /* art_Framework_Principal_Event_h */

// Local Variables:
// mode: c++
// End:
