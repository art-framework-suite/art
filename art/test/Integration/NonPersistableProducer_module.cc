////////////////////////////////////////////////////////////////////////
// Class:       NonPersistableProducer
// Plugin Type: producer (art v2_09_03)
// File:        NonPersistableProducer_module.cc
//
// Generated at Tue Jan  2 14:57:40 2018 by Kyle Knoepfel using cetskelgen
// from cetlib version v3_01_03.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/test/TestObjects/ToyProducts.h"

#include <memory>
#include <vector>

namespace art {
  namespace test {
    class NonPersistableProducer;
  }
}

class art::test::NonPersistableProducer : public EDProducer {
public:
  struct Config {
  };
  using Parameters = Table<Config>;

  explicit NonPersistableProducer(Parameters const&)
  {
    produces<arttest::PtrToNonPersistable>();
  }

  // Plugins should not be copied or assigned.
  NonPersistableProducer(NonPersistableProducer const&) = delete;
  NonPersistableProducer(NonPersistableProducer&&) = delete;
  NonPersistableProducer& operator=(NonPersistableProducer const&) = delete;
  NonPersistableProducer& operator=(NonPersistableProducer&&) = delete;

private:
  void
  produce(art::Event& e) override
  {
    auto non_persistable_name =
      std::make_unique<arttest::PtrToNonPersistable>("Billy");
    e.put(move(non_persistable_name));
  }
};

DEFINE_ART_MODULE(art::test::NonPersistableProducer)
