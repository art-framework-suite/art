#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "art/Utilities/ConfigTable.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include "fhiclcpp/types/Atom.h"
#include <cassert>

namespace arttest {
   class TestOutput;
}

class arttest::TestOutput : public art::OutputModule {
public:

  struct Config{
    fhicl::TableFragment<art::OutputModule::Config> omConfig;
    fhicl::Atom<int> shouldPass { fhicl::Name("shouldPass") };
  };

  using Parameters = art::ConfigTable<Config, art::OutputModule::Config::KeysToIgnore>;
  explicit TestOutput(Parameters const&);
  virtual ~TestOutput();

private:
   void write(art::EventPrincipal & e) override;
   void writeSubRun(art::SubRunPrincipal &) override {}
   void writeRun(art::RunPrincipal &) override {}
   void endJob() override;

   int num_pass_;
   int total_;
};

arttest::TestOutput::TestOutput(arttest::TestOutput::Parameters const& ps):
  art::OutputModule(ps().omConfig, ps.get_PSet()),
  num_pass_(ps().shouldPass()),
  total_(0)
{
}

arttest::TestOutput::~TestOutput()
{
}

void arttest::TestOutput::write(art::EventPrincipal &)
{
   ++total_;
   assert(currentContext() != 0);
}

void arttest::TestOutput::endJob()
{
   assert( currentContext() == 0 );
   mf::LogAbsolute("TestOutputReport")
      << "TestOutput: should pass " << num_pass_
      << ", did pass " << total_ << "\n";

   if(total_!=num_pass_) {
      throw cet::exception("TestOutputFailure")
         << "Number passed should be " << num_pass_
         << ", but got " << total_ << "\n";
   }
}

DEFINE_ART_MODULE(arttest::TestOutput)
