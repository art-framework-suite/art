#include "art/Framework/Principal/EventPrincipal.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/Principal.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <utility>

using namespace std;

namespace art {

class EventAuxiliary;
class ProcessConfiguration;
class History;
class DelayedReader;

EventPrincipal::
EventPrincipal(EventAuxiliary const& aux,
               ProcessConfiguration const& pc,
               ProductList const& productList,
               cet::exempt_ptr<ProductTable const> presentProducts,
               std::unique_ptr<History>&& history /*= std::make_unique<History>()*/,
               std::unique_ptr<DelayedReader>&& reader /*= std::make_unique<NoDelayedReader>()*/,
               bool const lastInSubRun /*= false*/)
  : Principal{aux, pc, productList, presentProducts, move(history), move(reader), lastInSubRun}
{
}
} // namespace art
