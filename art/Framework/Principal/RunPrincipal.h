#ifndef art_Framework_Principal_RunPrincipal_h
#define art_Framework_Principal_RunPrincipal_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

  class ProcessConfiguration;

  class RunPrincipal final : public Principal {

  public:
    using Auxiliary = RunAuxiliary;
    static constexpr BranchType branch_type = RunAuxiliary::branch_type;

  public:
    ~RunPrincipal();
    RunPrincipal(
      RunAuxiliary const&,
      ProcessConfiguration const&,
      cet::exempt_ptr<ProductTable const>,
      std::unique_ptr<DelayedReader>&& = std::make_unique<NoDelayedReader>());
  };

} // namespace art

// Local Variables:
// mode: c++
// End:
#endif /* art_Framework_Principal_RunPrincipal_h */
