#include "art/Framework/Principal/ResultsPrincipal.h"
// vim: set sw=2:

using namespace std;

namespace art {

  ResultsPrincipal::~ResultsPrincipal() {}

  ResultsPrincipal::ResultsPrincipal(
    ResultsAuxiliary const& aux,
    ProcessConfiguration const& pc,
    cet::exempt_ptr<ProductTable const> presentProducts,
    unique_ptr<DelayedReader>&& reader
    /*make_unique<NoDelayedReader>()*/)
    : Principal{aux, pc, presentProducts, move(reader)}
  {}

} // art
