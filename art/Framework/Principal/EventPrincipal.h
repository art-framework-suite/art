#ifndef art_Framework_Principal_EventPrincipal_h
#define art_Framework_Principal_EventPrincipal_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/EventAuxiliary.h"
#include "canvas/Persistency/Provenance/History.h"
#include "canvas/Persistency/Provenance/ProductTables.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

  class EventPrincipal final : public Principal {
  public:
    using Auxiliary = EventAuxiliary;
    static constexpr BranchType branch_type = Auxiliary::branch_type;

    ~EventPrincipal();
    EventPrincipal(
      EventAuxiliary const& aux,
      ProcessConfiguration const& pc,
      cet::exempt_ptr<ProductTable const> presentProducts,
      std::unique_ptr<History>&& history = std::make_unique<History>(),
      std::unique_ptr<DelayedReader>&& rtrv =
        std::make_unique<NoDelayedReader>(),
      bool lastInSubRun = false);

    EventAuxiliary const& eventAux() const;
    EventID const& eventID() const;
    EventNumber_t event() const;
    SubRunNumber_t subRun() const;
    RunNumber_t run() const;

    Timestamp const& time() const;

    SubRunPrincipal const& subRunPrincipal() const;
    void setSubRunPrincipal(cet::exempt_ptr<SubRunPrincipal const> srp);
    EventAuxiliary::ExperimentType ExperimentType() const;
    bool isReal() const;
    bool isLastInSubRun() const;

    void createGroupsForProducedProducts(ProductTables const& producedProducts);

  private:
    cet::exempt_ptr<SubRunPrincipal const> subRunPrincipal_{nullptr};
    EventAuxiliary aux_;
    std::unique_ptr<History> history_;
    bool lastInSubRun_;
  };

} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Principal_EventPrincipal_h */
