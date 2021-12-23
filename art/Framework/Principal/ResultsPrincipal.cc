#include "art/Framework/Principal/ResultsPrincipal.h"
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
                move(reader)}
    , aux_{aux}
  {}

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
