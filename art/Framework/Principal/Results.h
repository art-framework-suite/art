#ifndef art_Framework_Principal_Results_h
#define art_Framework_Principal_Results_h
// vim: set sw=2 expandtab :

// ==================================================================
//  This is the primary interface for accessing results-level
//  EDProducts and inserting new results-level EDProducts.
//
//  For its usage, see "art/Framework/Principal/DataViewImpl.h"
// ==================================================================

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Common/traits.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <utility>

namespace art {

  class Results final : private DataViewImpl {
  public:
    ~Results();

    explicit Results(ResultsPrincipal const& p, ModuleContext const& mc);

    Results(Results const&) = delete;
    Results(Results&&) = delete;
    Results& operator=(Results const&) = delete;
    Results& operator=(Results&&) = delete;

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

#endif /* art_Framework_Principal_Results_h */

// Local Variables:
// mode: c++
// End:
