// ======================================================================
//
// Prescaler_plugin
//
// ======================================================================

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/Frameworkfwd.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/types/Atom.h"

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
  bool filter(Event&, ScheduleID) override;

  size_t count_{};
  size_t const n_; // accept one in n
  size_t const
    offset_; // with offset,
             // i.e. sequence of events does not have to start at first event

}; // Prescaler

// ======================================================================

Prescaler::Prescaler(Parameters const& config)
  : SharedFilter{config}
  , n_{config().prescaleFactor()}
  , offset_{config().prescaleOffset()}
{
  // See note below.
  serialize();
}

bool
Prescaler::filter(Event&, ScheduleID)
{
  // The combination of incrementing, modulo dividing, and equality
  // comparing must be synchronized.  Changing count_ to the type
  // std::atomic<size_t> would not help since the entire combination
  // of operations must be atomic.
  return ++count_ % n_ == offset_;
}

DEFINE_ART_MODULE(Prescaler)
