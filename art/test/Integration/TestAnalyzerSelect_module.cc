// vim: set sw=2 expandtab :
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib_except/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace art;

namespace {

  using namespace fhicl;
  struct Config {
    Atom<int> shouldPass{Name("shouldPass")};
  };

} // unnamed namespace

namespace arttest {

  class TestAnalyzerSelect : public EDAnalyzer {
  public:
    using Parameters = Table<Config>;
    explicit TestAnalyzerSelect(Parameters const& ps)
      : EDAnalyzer{ps}, num_pass_{ps().shouldPass()}
    {}

  private:
    void
    analyze(const Event&) override
    {
      ++total_;
    }

    void endJob() override;

    int const num_pass_;
    int total_{};
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
