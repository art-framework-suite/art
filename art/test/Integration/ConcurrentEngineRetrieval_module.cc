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
#include "cetlib/SimultaneousFunctionSpawner.h"
#include "cetlib/test_macros.h"

#include "CLHEP/Random/RandFlat.h"

#include <algorithm>
#include <functional>
#include <vector>

namespace art {
  namespace test {

    class ConcurrentEngineRetrieval : public EDAnalyzer {
    public:
      explicit ConcurrentEngineRetrieval(fhicl::ParameterSet const& p);

      void analyze(art::Event const&) override;

    private:
      void generateNumbersDirectly();
      void generateNumbersIndirectly();

      template <typename F>
      void launchAndTest(std::vector<CLHEP::RandFlat>& dists, F f);

      ServiceHandle<RandomNumberGenerator> rng_{};
      std::vector<CLHEP::RandFlat> dists_{};
    };

    ConcurrentEngineRetrieval::ConcurrentEngineRetrieval(
      fhicl::ParameterSet const& p)
      : EDAnalyzer{p}
    {
      auto const seed = p.get<int>("seed", 0);
      std::uint16_t constexpr num_schedules{4};
      rng_->expandToNSchedules(
        num_schedules); // MT-TODO: To be removed whenever we
                        // are truly multi-schedule aware.
      for (std::uint16_t i{}; i < num_schedules; ++i) {
        dists_.emplace_back(rng_->createEngine(ScheduleID{i}, seed));
      }
    }

    void
    ConcurrentEngineRetrieval::analyze(art::Event const&)
    {
      generateNumbersDirectly();
      generateNumbersIndirectly();
    }

    template <typename F>
    void
    ConcurrentEngineRetrieval::launchAndTest(
      std::vector<CLHEP::RandFlat>& dists,
      F f)
    {
      std::vector<std::vector<std::size_t>> numsPerThread(dists.size());
      std::vector<std::function<void()>> tasks;
      std::size_t constexpr nums_size{5};
      std::size_t i{};
      for (auto& d : dists) {
        auto& nums = numsPerThread[i];
        nums.assign(nums_size, 0);
        tasks.emplace_back(std::bind(f, i, std::ref(d), std::ref(nums)));
        ++i;
      }
      cet::SimultaneousFunctionSpawner launch{tasks};

      // Check that all generated numbers per thread are the same
      for (std::uint16_t i{1}; i < numsPerThread.size(); ++i) {
        CET_CHECK_EQUAL_COLLECTIONS(numsPerThread[i], numsPerThread.front());
      }
    }

    // 'generateNumbersDirectly' does not call
    // RandomNumberGenerator::getEngine, but directly uses the
    // class-member distributions that were created in the c'tor.
    // This function tests the thread-safety of THIS module, and NOT
    // the RandomNumberGenerator service.
    void
    ConcurrentEngineRetrieval::generateNumbersDirectly()
    {
      auto generate_number =
        [](ScheduleID::size_type const, auto& d, auto& nums) {
          std::size_t constexpr random_range{1000};
          std::generate_n(nums.begin(), nums.size(), [&d] {
            return d.fireInt(random_range);
          });
        };
      launchAndTest(dists_, generate_number);
    }

    // 'generateNumbersIndirectly' DOES call
    // RandomNumberGenerate::getEngine in a multi-threaded context.
    // It tests BOTH the thread-safety of this module AND the
    // RandomNumberGenerator::getEngine facility.
    void
    ConcurrentEngineRetrieval::generateNumbersIndirectly()
    {
      std::vector<CLHEP::RandFlat> dists;
      auto generate_number =
        [this](ScheduleID::size_type const i, auto& d, auto& nums) {
          d = rng_->getEngine(ScheduleID{i});
          std::size_t constexpr random_range{1000};
          std::generate_n(nums.begin(), nums.size(), [&d] {
            return d.fireInt(random_range);
          });
        };
      launchAndTest(dists, generate_number);
    }

  } // namespace test
} // namespace art

DEFINE_ART_MODULE(art::test::ConcurrentEngineRetrieval)
