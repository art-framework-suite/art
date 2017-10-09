#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace art;

namespace {

  using namespace fhicl;
  struct Config {
    Atom<int> shouldPass{Name("shouldPass")};
  };
}

namespace arttest {

  class TestAnalyzerSelect : public EDAnalyzer {
  private:
    int num_pass_;
    int total_;

  public:
    using Parameters = Table<Config>;
    explicit TestAnalyzerSelect(Table<Config> const& ps)
      : EDAnalyzer(ps), num_pass_(ps().shouldPass()), total_(0)
    {}

    void
    analyze(const Event&) override
    {
      ++total_;
    }
    void endJob() override;
  };

  void
  TestAnalyzerSelect::endJob()
  {
    mf::LogAbsolute("TestAnalyzerSelectReport")
      << "TestAnalyzerSelect: should pass " << num_pass_ << ", did pass "
      << total_ << "\n";
    if (total_ != num_pass_) {
      throw cet::exception("TestAnalyzerExceptFailure")
        << "Number passed should be " << num_pass_ << ", but got " << total_
        << "\n";
    }
  }

} // namespace arttest

DEFINE_ART_MODULE(arttest::TestAnalyzerSelect)
