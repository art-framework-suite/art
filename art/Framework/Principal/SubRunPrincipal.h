#ifndef art_Framework_Principal_SubRunPrincipal_h
#define art_Framework_Principal_SubRunPrincipal_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

  class SubRunPrincipal final : public Principal {
  public:
    using Auxiliary = SubRunAuxiliary;
    static constexpr BranchType branch_type = Auxiliary::branch_type;

    ~SubRunPrincipal();
    SubRunPrincipal(
      SubRunAuxiliary const&,
      ProcessConfiguration const&,
      cet::exempt_ptr<ProductTable const>,
      std::unique_ptr<DelayedReader>&& = std::make_unique<NoDelayedReader>());

    SubRun makeSubRun(ModuleContext const& mc,
                      RangeSet const& rs = RangeSet::invalid());
    SubRun makeSubRun(ModuleContext const& mc) const;

    Timestamp const& beginTime() const;
    Timestamp const& endTime() const;
    RunPrincipal const& runPrincipal() const;
    RunID const& runID() const;
    RunNumber_t run() const;
    SubRunAuxiliary const& subRunAux() const;
    SubRunNumber_t subRun() const;
    SubRunID subRunID() const;
    using Principal::updateSeenRanges;

    void setRunPrincipal(cet::exempt_ptr<RunPrincipal const> rp);
    void createGroupsForProducedProducts(ProductTables const& producedProducts);

  private:
    cet::exempt_ptr<RunPrincipal const> runPrincipal_{nullptr};
    SubRunAuxiliary aux_;
  };

} // namespace art

#endif /* art_Framework_Principal_SubRunPrincipal_h */

// Local Variables:
// mode: c++
// End:
