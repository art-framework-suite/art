#include "test/Framework/Services/Optional/PSTestInterfaceImpl.h"

arttest::PSTestInterfaceImpl::PSTestInterfaceImpl(fhicl::ParameterSet const &,
                                                  art::ActivityRegistry &,
                                                  art::ScheduleID sID)
  :
  PSTestInterface(),
  sID_(sID)
{
}

DEFINE_ART_SERVICE_INTERFACE_IMPL(arttest::PSTestInterfaceImpl, arttest::PSTestInterface)
