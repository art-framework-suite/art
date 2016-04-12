#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/test/Integration/Wanted.h"

arttest::Wanted::Wanted(fhicl::ParameterSet const &,
                        art::ActivityRegistry & r):
  postBeginJobCalled_(false)
{
  r.sPostBeginJob.watch(this, &Wanted::postBeginJob);
}


void
arttest::Wanted::postBeginJob()
{
  postBeginJobCalled_ = true;
}

DEFINE_ART_SERVICE(arttest::Wanted)

