#include "art/Framework/Principal/ResultsPrincipal.h"
// vim: set sw=2:

art::ResultsPrincipal::ResultsPrincipal(
  ResultsAuxiliary const& aux,
  ProcessConfiguration const& pc,
  cet::exempt_ptr<ProductTable const> presentProducts,
  std::unique_ptr<DelayedReader>&&
    reader /*std::make_unique<NoDelayedReader>()*/)
  : Principal{aux, pc, presentProducts, move(reader)}
{}
