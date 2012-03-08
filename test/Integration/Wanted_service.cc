#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/Integration/Wanted.h"

arttest::Wanted::Wanted(fhicl::ParameterSet const &,
                        art::ActivityRegistry & r):
  postBeginJobCalled_(false)
{
  r.watchPostBeginJob(this, &Wanted::postBeginJob);
}


void
arttest::Wanted::postBeginJob()
{
  postBeginJobCalled_ = true;
}

DEFINE_ART_SERVICE(arttest::Wanted)

