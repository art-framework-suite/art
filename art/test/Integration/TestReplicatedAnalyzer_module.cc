#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Utilities/ScheduleID.h"

#include <iostream>

using namespace art;

namespace {
  class TestReplicatedAnalyzer : public ReplicatedAnalyzer {
  public:
    struct Config {};
    using Parameters = Table<Config>;
    explicit TestReplicatedAnalyzer(Parameters const& p,
                                    ScheduleID const sid) noexcept
      : ReplicatedAnalyzer{p}, sid_{sid}
    {
      std::cout << "Module constructor - ScheduleID: " << sid_ << '\n';
    }
  private:
    void analyze(Event const&) override;
    ScheduleID const sid_;
  };

  void
  TestReplicatedAnalyzer::analyze(Event const& e)
  {
    std::cout << "Schedule: " << sid_ << " Event: " << e.id() << '\n';
  }
}

DEFINE_ART_MODULE(TestReplicatedAnalyzer)
