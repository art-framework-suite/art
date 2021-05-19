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

namespace art {

  class Run final : private DataViewImpl {

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~Run();

    explicit Run(RunPrincipal const& principal,
                 ModuleContext const& mc,
                 RangeSet const& rs = RangeSet::invalid());

    Run(Run const&) = delete;
    Run(Run&&) = delete;
    Run& operator=(Run const&) = delete;
    Run& operator=(Run&&) = delete;

    RunID id() const;
    using DataViewImpl::beginTime;
    using DataViewImpl::endTime;
    using DataViewImpl::run;

    using DataViewImpl::getHandle;
    using DataViewImpl::getInputTags;
    using DataViewImpl::getMany;
    using DataViewImpl::getProduct;
    using DataViewImpl::getProductTokens;
    using DataViewImpl::getValidHandle;
    using DataViewImpl::getView;
    using DataViewImpl::put;

    using DataViewImpl::getProductDescription;
    using DataViewImpl::getProductID;

    using DataViewImpl::getProcessParameterSet;
    using DataViewImpl::processHistory;
    using DataViewImpl::productGetter;
    using DataViewImpl::removeCachedProduct;

    // Obsolete interface (will be deprecated)
    using DataViewImpl::get;
    using DataViewImpl::getByLabel;
    using DataViewImpl::getByToken;
    using DataViewImpl::getManyByType;
    using DataViewImpl::getPointerByLabel;

    using DataViewImpl::movePutProductsToPrincipal;
  };

} // namespace art

#endif /* art_Framework_Principal_Run_h */

// Local Variables:
// mode: c++
// End:
