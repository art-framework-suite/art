#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/Globals.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"

namespace {
  class ScheduleThreadChecker : public art::EDAnalyzer {
  public:
    struct Config {
      fhicl::Atom<unsigned> expected_schedules{fhicl::Name{"num_schedules"}};
      fhicl::Atom<unsigned> expected_threads{fhicl::Name{"num_threads"}};
    };
    using Parameters = Table<Config>;
    explicit ScheduleThreadChecker(Parameters const& p);

  private:
    void
    analyze(art::Event const&) override
    {}
  };

  ScheduleThreadChecker::ScheduleThreadChecker(Parameters const& p)
    : EDAnalyzer{p}
  {
    auto const* globals = art::Globals::instance();
    auto const nschedules = globals->nschedules();
    auto const nthreads = globals->nthreads();
    if (p().expected_schedules() != nschedules or
        p().expected_threads() != nthreads) {
      throw art::Exception{
        art::errors::LogicError,
        "The expected number of threads and schedules does not match those "
        "that are actually configured for this job.\n"}
        << "Expected: " << p().expected_threads() << " threads and "
        << p().expected_schedules() << " schedules\n"
        << "Actual  : " << nthreads << " threads and " << nschedules
        << " schedules\n";
    }
  }
}

DEFINE_ART_MODULE(ScheduleThreadChecker)
