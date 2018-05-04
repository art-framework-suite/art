#ifndef art_Framework_Principal_Run_h
#define art_Framework_Principal_Run_h
// vim: set sw=2 expandtab :

//
//  This is the primary interface for accessing per run EDProducts
//  and inserting new derived products.
//
//  For its usage, see "art/Framework/Principal/DataViewImpl.h"
//

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Utilities/TypeID.h"

namespace art {

  class Run final : private DataViewImpl {

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~Run();

    explicit Run(RunPrincipal const&,
                 ModuleDescription const&,
                 RangeSet const& rs = RangeSet::invalid());

    Run(Run const&) = delete;
    Run(Run&&) = delete;
    Run& operator=(Run const&) = delete;
    Run& operator=(Run&&) = delete;

    RunID id() const;

    using DataViewImpl::get;
    using DataViewImpl::getByLabel;
    using DataViewImpl::getByToken;
    using DataViewImpl::getValidHandle;
    using DataViewImpl::getView;
    using DataViewImpl::getMany;
    using DataViewImpl::getManyByType;
    using DataViewImpl::put;

    using DataViewImpl::removeCachedProduct;
    using DataViewImpl::getPointerByLabel;
    using DataViewImpl::getProductID;
    using DataViewImpl::productGetter;

    using DataViewImpl::run;
    using DataViewImpl::beginTime;
    using DataViewImpl::endTime;
    using DataViewImpl::movePutProductsToPrincipal;
  };

} // namespace art

#endif /* art_Framework_Principal_Run_h */

// Local Variables:
// mode: c++
// End:
