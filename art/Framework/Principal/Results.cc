#include "art/Framework/Principal/Results.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ResultsPrincipal.h"

namespace art {

  Results::~Results() = default;

  Results
  Results::make(ResultsPrincipal& p, ModuleContext const& mc)
  {
    return Results{
      p, mc, std::make_optional<ProductInserter>(InResults, p, mc)};
  }

  Results
  Results::make(ResultsPrincipal const& p, ModuleContext const& mc)
  {
    return Results{p, mc, std::nullopt};
  }

  Results::Results(ResultsPrincipal const& p,
                   ModuleContext const& mc,
                   std::optional<ProductInserter> inserter)
    : ProductRetriever{InResults, p, mc, false}, inserter_{move(inserter)}
  {}

  void
  Results::commitProducts()
  {
    assert(inserter_);
    inserter_->commitProducts();
  }

} // namespace art
