//========================================================================
// ConcurrentEngineRetrieval
//
// This is module that is intended to test the parallel retrieval/use
// of a RandomNumberGenerator engine.  Two tests are performed:
//
//  (1) Create multiple distributions in the c'tor, but do not
//      directly call RandomNumberGenerator::getEngine during the
//      testing of the random-number-generator.
//
//  (2) Create multiple distributions simultaneously be calling
//      RandomNumberGenerator::getEngine in multiple threads.
// ========================================================================

#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/ScheduleIteration.h"
#include "cetlib/SimultaneousFunctionSpawner.h"
#include "cetlib/test_macros.h"

#include "CLHEP/Random/RandFlat.h"

#include <algorithm>
#include <functional>
#include <vector>

namespace art {
  namespace test {

    class ConcurrentEngineRetrieval : public ReplicatedAnalyzer {
    public:
      explicit ConcurrentEngineRetrieval(fhicl::ParameterSet const& p,
                                         ScheduleID);

    private:
      void analyze(art::Event const&) override;
      CLHEP::RandFlat dist_;
    };

    ConcurrentEngineRetrieval::ConcurrentEngineRetrieval(
      fhicl::ParameterSet const& p,
      ScheduleID const sid)
      : ReplicatedAnalyzer{p}, dist_{createEngine(sid, p.get<int>("seed", 0))}
    {}

    void
    ConcurrentEngineRetrieval::analyze(art::Event const&)
    {
      std::vector<std::size_t> numbers(5);
      std::size_t constexpr random_range{1000};
      std::generate_n(numbers.begin(), numbers.size(), [this] {
        return dist_.fireInt(random_range);
      });
      // Find way to verify this for all schedules?
    }

  } // namespace test
} // namespace art

DEFINE_ART_MODULE(art::test::ConcurrentEngineRetrieval)
