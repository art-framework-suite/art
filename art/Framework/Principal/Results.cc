#include "art/Framework/Principal/Results.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ResultsPrincipal.h"
#include "canvas/Persistency/Provenance/BranchType.h"

namespace art {

  Results::~Results() = default;

  Results::Results(ResultsPrincipal const& p, ModuleContext const& mc)
    : DataViewImpl{InResults, p, mc, false, RangeSet::invalid()}
  {}

} // namespace art
