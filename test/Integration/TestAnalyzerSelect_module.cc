#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace art;

namespace arttest {

class TestAnalyzerSelect : public EDAnalyzer {
private:
  int num_pass_;
  int total_;
public:
  explicit TestAnalyzerSelect(fhicl::ParameterSet const& ps)
    : EDAnalyzer(ps), num_pass_(ps.get<int>("shouldPass")), total_(0) {}
  ~TestAnalyzerSelect() {}
  void analyze(const Event&) { ++total_; }
  void endJob();
};

void TestAnalyzerSelect::endJob()
{
  mf::LogAbsolute("TestAnalyzerSelectReport")
      << "TestAnalyzerSelect: should pass " << num_pass_
      << ", did pass " << total_ << "\n";
  if (total_ != num_pass_) {
    throw cet::exception("TestAnalyzerExceptFailure")
        << "Number passed should be " << num_pass_
        << ", but got " << total_ << "\n";
  }
}

} // namespace arttest

DEFINE_ART_MODULE(arttest::TestAnalyzerSelect)
