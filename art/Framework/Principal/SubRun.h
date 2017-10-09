#ifndef art_Framework_Principal_SubRun_h
#define art_Framework_Principal_SubRun_h
// vim: set sw=2 expandtab :

//
//  This is the primary interface for accessing per subRun
//  EDProducts and inserting new derived per subRun EDProducts.
//
//  For its usage, see "art/Framework/Principal/DataViewImpl.h"
//

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "canvas/Persistency/Provenance/SubRunID.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <utility>

namespace art {

  class SubRun final : public DataViewImpl {

  public:
    ~SubRun();

    SubRun(SubRunPrincipal const& srp,
           ModuleDescription const& md,
           RangeSet const& rs = RangeSet::invalid());

    SubRun(SubRun const&) = delete;

    SubRun(SubRun&&) = delete;

    SubRun& operator=(SubRun const&) = delete;

    SubRun& operator=(SubRun&&) = delete;

  public:
    SubRunID id() const;

    Run const& getRun() const;

  private:
    std::unique_ptr<Run const> const run_;
  };

} // namespace art

#endif /* art_Framework_Principal_SubRun_h */

// Local Variables:
// mode: c++
// End:
