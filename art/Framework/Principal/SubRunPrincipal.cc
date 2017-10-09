#include "art/Framework/Principal/SubRunPrincipal.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

  class SubRunAuxiliary;
  class ProcessConfiguration;
  class DelayedReader;

  SubRunPrincipal::SubRunPrincipal(
    SubRunAuxiliary const& aux,
    ProcessConfiguration const& pc,
    cet::exempt_ptr<ProductTable const> presentProducts,
    std::unique_ptr<DelayedReader>&&
      reader /*= std::make_unique<NoDelayedReader>()*/)
    : Principal{aux, pc, presentProducts, move(reader)}
  {}

} // namespace art
