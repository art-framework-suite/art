#include "art/Framework/Principal/ResultsPrincipal.h"
// vim: set sw=2:

art::ResultsPrincipal::
ResultsPrincipal(ResultsAuxiliary const& aux,
                 ProcessConfiguration const& pc,
                 ProductList const& productList,
                 cet::exempt_ptr<ProductTable const> presentProducts,
                 std::unique_ptr<DelayedReader>&& reader /*std::make_unique<NoDelayedReader>()*/)
  : Principal{aux, pc, productList, presentProducts, move(reader)}
{
}
