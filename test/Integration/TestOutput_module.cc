#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/OutputModule.h"
#include "cetlib/exception.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>

namespace arttest {
   class TestOutput;
}

class arttest::TestOutput : public art::OutputModule {
public:
   explicit TestOutput(fhicl::ParameterSet const&);
   virtual ~TestOutput();

private:
   virtual void write(art::EventPrincipal const& e);
   virtual void writeSubRun(art::SubRunPrincipal const&){}
   virtual void writeRun(art::RunPrincipal const&){}
   virtual void endJob();

   int num_pass_;
   int total_;
};

arttest::TestOutput::TestOutput(fhicl::ParameterSet const& ps):
   art::OutputModule(ps),
   num_pass_(ps.get<int>("shouldPass")),
   total_(0)
{
}

arttest::TestOutput::~TestOutput()
{
}

void arttest::TestOutput::write(art::EventPrincipal const&)
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

DEFINE_ART_MODULE(arttest::TestOutput);
