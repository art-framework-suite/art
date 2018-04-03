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
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/types/Atom.h"

#include <memory>
#include <mutex>
#include <vector>

using namespace std;
using namespace fhicl;

namespace art {

  class RandomNumberSaver : public EDProducer {

  public: // CONFIGURATION
    struct Config {

      Atom<bool> debug{Name{"debug"}, false};
    };

    using Parameters = EDProducer::Table<Config>;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    explicit RandomNumberSaver(Parameters const&);

  public: // MEMBER FUNCTIONS -- API required by EDProducer
    void produce(Event&) override;

  private: // MEMBER DATA
    // When true makes produce call rng->print_().
    bool debug_;

    // Used only when debug_ == true to serialize
    // usage of rng->print_().
    mutex m_{};
  };

  RandomNumberSaver::RandomNumberSaver(Parameters const& config)
    : debug_{config().debug()}, m_{}
  {
    produces<vector<RNGsnapshot>>();
  }

  void
  RandomNumberSaver::produce(Event& e)
  {
    ServiceHandle<RandomNumberGenerator const> rng;
    e.put(make_unique<vector<RNGsnapshot>>(rng->accessSnapshot_(scheduleID())));
    if (debug_) {
      // Only take out the lock if running in debug mode.
      lock_guard<mutex> hold{m_};
      rng->print_();
    }
  }

} // namespace art

DEFINE_ART_MODULE(art::RandomNumberSaver)
