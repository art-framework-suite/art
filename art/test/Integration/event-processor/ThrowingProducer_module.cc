#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/test/Integration/event-processor/ThrowAfterConfig.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/types/Atom.h"

namespace art {
  namespace test {

    class ThrowingProducer : public EDProducer {
      unsigned count_{};
      unsigned const threshold_;

    public:
      using Parameters = Table<ThrowAfterConfig>;

      ThrowingProducer(Parameters const& p)
        : EDProducer{p}, threshold_{p().throwAfter()}
      {
        if (p().throwFromCtor()) {
          throw Exception{errors::OtherArt} << "Throw from c'tor.\n";
        }
      }

    private:
      void
      produce(Event&) override
      {
        ++count_;
        if (count_ >= threshold_) {
          throw Exception{errors::OtherArt} << "Throw from produce.\n";
        }
      }
    };

  } // namespace test
} // namespace art

DEFINE_ART_MODULE(art::test::ThrowingProducer)
