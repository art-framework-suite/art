#ifndef art_Framework_Principal_SubRunPrincipal_h
#define art_Framework_Principal_SubRunPrincipal_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/SubRunAuxiliary.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

  class ProcessConfiguration;

  class SubRunPrincipal final : public Principal {

  public:
    using Auxiliary = SubRunAuxiliary;
    static constexpr BranchType branch_type = Auxiliary::branch_type;

  public:
    SubRunPrincipal(
      SubRunAuxiliary const&,
      ProcessConfiguration const&,
      cet::exempt_ptr<ProductTable const>,
      std::unique_ptr<DelayedReader>&& = std::make_unique<NoDelayedReader>());
  };

} // namespace art

#endif /* art_Framework_Principal_SubRunPrincipal_h */

// Local Variables:
// mode: c++
// End:
