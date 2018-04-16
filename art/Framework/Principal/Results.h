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

  class Results final : public DataViewImpl {

  public:
    ~Results();

    explicit Results(ResultsPrincipal const& p, ModuleDescription const& md);

    Results(Results const&) = delete;
    Results(Results&&) = delete;
    Results& operator=(Results const&) = delete;
    Results& operator=(Results&&) = delete;
  };

} // namespace art

#endif /* art_Framework_Principal_Results_h */

// Local Variables:
// mode: c++
// End:
