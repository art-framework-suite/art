#include "art/Framework/Principal/ResultsPrincipal.h"
// vim: set sw=2:

#include "art/Framework/Principal/Principal.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <utility>

using namespace std;

namespace art {

class DelayedReader;
class ProcessConfiguration;
class ResultsAuxiliary;

ResultsPrincipal::
~ResultsPrincipal()
{
}

ResultsPrincipal::
ResultsPrincipal(ResultsAuxiliary const& aux, ProcessConfiguration const& pc,
                 std::unique_ptr<DelayedReader>&& reader /*std::make_unique<NoDelayedReader>()*/)
  : Principal{aux, pc, move(reader)}
{
}

} // namespace art

