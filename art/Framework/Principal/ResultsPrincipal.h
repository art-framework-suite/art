#ifndef art_Framework_Principal_ResultsPrincipal_h
#define art_Framework_Principal_ResultsPrincipal_h
// vim: set sw=2 expandtab :

//
//  ResultsPrincipal
//
//  Manages per-file results data products.
//
//  This is not visible to modules, instead they use the Results class,
//  which is a proxy for this class.
//

#include "art/Framework/Principal/NoDelayedReader.h"
#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ResultsAuxiliary.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

class ProcessConfiguration;

class ResultsPrincipal final : public Principal {

public:

  using Auxiliary = ResultsAuxiliary;
  static constexpr BranchType branch_type = ResultsAuxiliary::branch_type;

public:

  ~ResultsPrincipal();

  ResultsPrincipal(ResultsAuxiliary const&, ProcessConfiguration const&,
                   std::unique_ptr<DelayedReader>&& reader = std::make_unique<NoDelayedReader>());

};

} // namespace art

#endif /* art_Framework_Principal_ResultsPrincipal_h */

// Local Variables:
// mode: c++
// End:
