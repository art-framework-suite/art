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
#include "canvas/Persistency/Provenance/fwd.h"
#include "cetlib/exempt_ptr.h"

#include <memory>

namespace art {

  class ResultsPrincipal final : public Principal {
  public:
    using Auxiliary = ResultsAuxiliary;
    static constexpr BranchType branch_type = ResultsAuxiliary::branch_type;

    ~ResultsPrincipal();
    ResultsPrincipal(ResultsAuxiliary const&,
                     ProcessConfiguration const&,
                     cet::exempt_ptr<ProductTable const> presentProducts,
                     std::unique_ptr<DelayedReader>&& reader =
                       std::make_unique<NoDelayedReader>());

    Results makeResults(ModuleContext const& mc);
    Results makeResults(ModuleContext const& mc) const;

    ResultsAuxiliary const& resultsAux() const;

    void createGroupsForProducedProducts(ProductTables const& producedProducts);

  private:
    ResultsAuxiliary aux_;
  };

} // namespace art

#endif /* art_Framework_Principal_ResultsPrincipal_h */

// Local Variables:
// mode: c++
// End:
