#include "art/Framework/Principal/ResultsPrincipal.h"
#include "art/Framework/Principal/Results.h"
// vim: set sw=2:

namespace art {

  ResultsPrincipal::~ResultsPrincipal() = default;

  ResultsPrincipal::ResultsPrincipal(
    ResultsAuxiliary const& aux,
    ProcessConfiguration const& pc,
    cet::exempt_ptr<ProductTable const> presentProducts,
    std::unique_ptr<DelayedReader>&& reader
    /*make_unique<NoDelayedReader>()*/)
    : Principal{InResults,
                pc,
                presentProducts,
                aux.processHistoryID(),
                std::move(reader)}
    , aux_{aux}
  {}

  Results
  ResultsPrincipal::makeResults(ModuleContext const& mc)
  {
    return Results{*this, mc, makeInserter(mc)};
  }

  Results
  ResultsPrincipal::makeResults(ModuleContext const& mc) const
  {
    return Results{*this, mc};
  }

  ResultsAuxiliary const&
  ResultsPrincipal::resultsAux() const
  {
    return aux_;
  }

  void
  ResultsPrincipal::createGroupsForProducedProducts(
    ProductTables const& producedProducts)
  {
    Principal::createGroupsForProducedProducts(producedProducts);
    aux_.setProcessHistoryID(processHistoryID());
  }
} // art
