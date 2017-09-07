#include "art/Framework/Principal/RunPrincipal.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Principal.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <utility>

using namespace std;

namespace art {

class DelayedReader;
class ProcessConfiguration;
class RunAuxiliary;

RunPrincipal::
~RunPrincipal()
{
}

RunPrincipal::
RunPrincipal(RunAuxiliary const& aux, ProcessConfiguration const& pc,
             std::unique_ptr<DelayedReader>&& reader /*= std::make_unique<NoDelayedReader>()*/)
  : Principal{aux, pc, move(reader)}
{
}

} // namespace art

