// ======================================================================
//
// Prescaler_plugin
//
// ======================================================================

#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedFilter.h"
#include "fhiclcpp/types/Atom.h"

#include <mutex>

namespace art {
  class Prescaler;
}
using namespace fhicl;
using art::Prescaler;

// ======================================================================

class art::Prescaler : public SharedFilter {
public:
  struct Config {
    Atom<size_t> prescaleFactor{Name("prescaleFactor")};
    Atom<size_t> prescaleOffset{Name("prescaleOffset")};
  };

  using Parameters = Table<Config>;
  explicit Prescaler(Parameters const&);

private:
  bool filter(Event&, Services const&) override;

  size_t count_{};
  // Accept one in n events.
  size_t const n_;
  // An offset is allowed--i.e. sequence of events does not have to
  // start at first event.
  size_t const offset_;
  std::mutex mutex_{};

}; // Prescaler

// ======================================================================

Prescaler::Prescaler(Parameters const& config)
  : SharedFilter{config}
  , n_{config().prescaleFactor()}
  , offset_{config().prescaleOffset()}
{
  async<InEvent>();
}

bool
Prescaler::filter(Event&, Services const&)
{
  // The combination of incrementing, modulo dividing, and equality
  // comparing must be synchronized.  Changing count_ to the type
  // std::atomic<size_t> would not help since the entire combination
  // of operations must be atomic.  Using a mutex here is cheaper than
  // calling serialize(), since that will also serialize any of the
  // module-level service callbacks invoked before and after this
  // function is called.
  std::lock_guard<std::mutex> lock{mutex_};
  ++count_;
  return count_ % n_ == offset_;
}

DEFINE_ART_MODULE(Prescaler)
