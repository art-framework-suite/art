// vim: set sw=2 expandtab :
//
// Store state of the RandomNumberGenerator service into the event.
//

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedProducer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/types/Atom.h"

#include <memory>
#include <vector>

using namespace std;
using namespace fhicl;

namespace art {

  class RandomNumberSaver : public SharedProducer {
  public:
    struct Config {
      Atom<bool> debug{Name{"debug"}, false};
    };

    using Parameters = Table<Config>;
    explicit RandomNumberSaver(Parameters const&);

  private:
    void produce(Event&, ScheduleID) override;
    // When true makes produce call rng->print_().
    bool const debug_;
  };

  RandomNumberSaver::RandomNumberSaver(Parameters const& config)
    : SharedProducer{config}, debug_{config().debug()}
  {
    produces<vector<RNGsnapshot>>();
    if (debug_) {
      // If debugging information is desired, serialize so that the
      // printing is not garbled.
      serialize<InEvent>();
    } else {
      async<InEvent>();
    }
  }

  void
  RandomNumberSaver::produce(Event& e, ScheduleID const sid)
  {
    ServiceHandle<RandomNumberGenerator const> rng;
    e.put(make_unique<vector<RNGsnapshot>>(rng->accessSnapshot_(sid)));
    if (debug_) {
      rng->print_();
    }
  }

} // namespace art

DEFINE_ART_MODULE(art::RandomNumberSaver)
