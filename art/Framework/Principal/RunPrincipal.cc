#include "art/Framework/Principal/RunPrincipal.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

class DelayedReader;
class ProcessConfiguration;
class RunAuxiliary;

RunPrincipal::
RunPrincipal(RunAuxiliary const& aux,
             ProcessConfiguration const& pc,
             ProductList const& productList,
             cet::exempt_ptr<ProductTable const> presentProducts,
             std::unique_ptr<DelayedReader>&& reader /*= std::make_unique<NoDelayedReader>()*/)
  : Principal{aux, pc, productList, presentProducts, move(reader)}
{
}

} // namespace art
