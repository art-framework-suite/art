#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/test/Integration/event-processor/ThrowAfterConfig.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/types/Atom.h"

namespace art {
  namespace test {
    class ThrowingAnalyzer;
  }
} // namespace art

class art::test::ThrowingAnalyzer : public EDAnalyzer {
  unsigned count_{};
  unsigned threshold_;

public:
  using Parameters = EDAnalyzer::Table<ThrowAfterConfig>;

  ThrowingAnalyzer(Parameters const& p)
    : EDAnalyzer{p}, threshold_{p().throwAfter()}
  {
    if (p().throwFromCtor()) {
      throw Exception{errors::OtherArt} << "Throw from c'tor.\n";
    }
  }

  void
  analyze(Event const&) override
  {
    if (++count_ > threshold_) {
      throw Exception{errors::OtherArt} << "Throw from analyze.\n";
    }
  }
};

DEFINE_ART_MODULE(art::test::ThrowingAnalyzer)
