// Service to make sure we can use another service.

#include "art/test/Integration/ServiceUsing.h"

art::test::ServiceUsing::ServiceUsing(fhicl::ParameterSet const&,
                                    art::ActivityRegistry& reg)
{
  reg.sPostBeginJob.watch(this, &ServiceUsing::postBeginJob);
  wanted_->setValue(cached_value_);
}

void art::test::ServiceUsing::postBeginJob()
{
  postBeginJobCalled_ = true;
  cached_value_ = 10;
  wanted_->setValue(cached_value_);
}

DEFINE_ART_SERVICE(art::test::ServiceUsing)
