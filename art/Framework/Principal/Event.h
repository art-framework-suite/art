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
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/GroupQueryResult.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/EventID.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"

#include <cstdlib>
#include <memory>
#include <set>
#include <vector>

namespace art {

  class BranchDescription;
  class ProdToProdMapBuilder;

  class Event final : private DataViewImpl {
  public:
    template <typename T>
    using HandleT = Handle<T>;

    ~Event();

    explicit Event(EventPrincipal const& ep, ModuleContext const& mc);

    Event(Event const&) = delete;
    Event(Event&&) = delete;
    Event& operator=(Event const&) = delete;
    Event& operator=(Event&&) = delete;

    EventID id() const;
    using DataViewImpl::event;
    using DataViewImpl::experimentType;
    using DataViewImpl::isRealData;
    using DataViewImpl::run;
    using DataViewImpl::subRun;
    using DataViewImpl::time;

    SubRun const& getSubRun() const;
    Run const& getRun() const;

    using DataViewImpl::history;
    using DataViewImpl::processHistoryID;

    using DataViewImpl::put;

    using DataViewImpl::get;
    using DataViewImpl::getByLabel;
    using DataViewImpl::getByToken;
    using DataViewImpl::getInputTags;
    using DataViewImpl::getMany;
    using DataViewImpl::getManyByType;
    using DataViewImpl::getPointerByLabel;
    using DataViewImpl::getValidHandle;
    using DataViewImpl::getView;

    using DataViewImpl::getProductDescription;
    using DataViewImpl::getProductID;

    using DataViewImpl::getProcessParameterSet;
    using DataViewImpl::processHistory;
    using DataViewImpl::productGetter;
    using DataViewImpl::removeCachedProduct;

    using DataViewImpl::movePutProductsToPrincipal;

  private:
    std::unique_ptr<SubRun const> const subRun_;
  };

} // namespace art

#endif /* art_Framework_Principal_Event_h */

// Local Variables:
// mode: c++
// End:
