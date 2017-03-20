// ======================================================================
//
// RandomNumberSaver_plugin:  Store state of the RandomNumberGenerator
//                            service into the event.
//
// ======================================================================

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/types/Atom.h"

#include <memory>
#include <mutex>

namespace art {
  class RandomNumberSaver;
}

using namespace fhicl;
using art::RandomNumberSaver;

// ======================================================================

class art::RandomNumberSaver : public EDProducer {
  using RNGservice = RandomNumberGenerator;

public:
  // --- Characteristics:
  using label_t    = RNGservice::label_t;
  using snapshot_t = RNGservice::snapshot_t;

  // --- Configuration
  struct Config {
    Atom<bool> debug {Name{"debug"}, false};
  };

  using Parameters = EDProducer::Table<Config>;
  explicit RandomNumberSaver(Parameters const&);

  void produce(Event&) override;

private:
  bool debug_;
  std::mutex m_ {};
};  // RandomNumberSaver

// ======================================================================

RandomNumberSaver::
RandomNumberSaver(Parameters const& config)
  : debug_{config().debug()}
{
  produces<snapshot_t>();
}

// ----------------------------------------------------------------------

void
RandomNumberSaver::produce(Event& e)
{
  // Placeholder until we can directly access the schedule ID.
  unsigned const scheduleID {0u};
  ServiceHandle<RNGservice const> rng;
  e.put(std::make_unique<snapshot_t>(rng->accessSnapshot_(scheduleID)));

  if (debug_) {
    // Only take out the lock if running in debug mode.
    std::lock_guard<std::mutex> hold {m_};
    rng->print_();
  }
}

DEFINE_ART_MODULE(RandomNumberSaver)
