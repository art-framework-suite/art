// vim: set sw=2 expandtab :
//
// Store state of the RandomNumberGenerator
// service into the event.
//

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/PerThread.h"
#include "fhiclcpp/types/Atom.h"

#include <memory>
#include <vector>

using namespace std;
using namespace fhicl;

namespace art {

  class RandomNumberSaver : public EDProducer {

    // Configuration
  public:
    struct Config {
      Atom<bool> debug{Name{"debug"}, false};
    };

    using Parameters = EDProducer::Table<Config>;

    // Special Member Functions
  public:
    explicit RandomNumberSaver(Parameters const&);

    // API required by EDProducer
  public:
    void produce(Event&) override;

    // Implementation details
  private:
    // When true makes produce call rng->print_().
    bool const debug_;
  };

  RandomNumberSaver::RandomNumberSaver(Parameters const& config)
    : debug_{config().debug()}
  {
    produces<vector<RNGsnapshot>>();
  }

  void
  RandomNumberSaver::produce(Event& e)
  {
    ServiceHandle<RandomNumberGenerator const> rng;
    e.put(make_unique<vector<RNGsnapshot>>(rng->accessSnapshot_(scheduleID())));
    if (debug_) {
      rng->print_();
    }
  }

} // namespace art

DEFINE_ART_MODULE(art::RandomNumberSaver)
