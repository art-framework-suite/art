#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Utilities/make_tool.h"
#include "art/test/Integration/Wanted.h"
#include "art/test/Integration/MessagePrinter.h"

#include "fhiclcpp/ParameterSet.h"

arttest::Wanted::Wanted(fhicl::ParameterSet const& pset,
                        art::ActivityRegistry& r) :
  postBeginJobCalled_{false}
{
  r.sPostBeginJob.watch(this, &Wanted::postBeginJob);
  // Test tool invocation from within service.
  auto mp = art::make_tool<MessagePrinter>(pset.get<fhicl::ParameterSet>("messagePrinter"));
  mp->print(std::cerr, "Calling a tool from inside of a service.\n");
}


void
arttest::Wanted::postBeginJob()
{
  postBeginJobCalled_ = true;

}

DEFINE_ART_SERVICE(arttest::Wanted)
