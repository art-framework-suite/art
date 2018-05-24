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
#include <set>
#include <utility>

namespace art {

  class SubRun final : private DataViewImpl {
  public:
    ~SubRun();

    explicit SubRun(SubRunPrincipal const& srp,
                    ModuleContext const& mc,
                    RangeSet const& rs = RangeSet::invalid());

    SubRun(SubRun const&) = delete;
    SubRun(SubRun&&) = delete;
    SubRun& operator=(SubRun const&) = delete;
    SubRun& operator=(SubRun&&) = delete;

    SubRunID id() const;
    using DataViewImpl::beginTime;
    using DataViewImpl::endTime;
    using DataViewImpl::run;
    using DataViewImpl::subRun;

    using DataViewImpl::get;
    using DataViewImpl::getByLabel;
    using DataViewImpl::getByToken;
    using DataViewImpl::getMany;
    using DataViewImpl::getManyByType;
    using DataViewImpl::getPointerByLabel;
    using DataViewImpl::getValidHandle;
    using DataViewImpl::getView;
    using DataViewImpl::put;

    Run const& getRun() const;
    using DataViewImpl::getProductDescription;
    using DataViewImpl::getProductID;

    using DataViewImpl::processHistory;
    using DataViewImpl::productGetter;
    using DataViewImpl::removeCachedProduct;

    using DataViewImpl::movePutProductsToPrincipal;

  private:
    std::unique_ptr<Run const> const run_;
  };

} // namespace art

#endif /* art_Framework_Principal_SubRun_h */

// Local Variables:
// mode: c++
// End:
