#include "test/Framework/Services/Optional/LocalServiceTestInterfaceImpl.h"

arttest::LocalServiceTestInterfaceImpl::LocalServiceTestInterfaceImpl(fhicl::ParameterSet const &,
                                                  art::ActivityRegistry &,
                                                  art::ScheduleID sID)
  :
  LocalServiceTestInterface(),
  sID_(sID)
{
}

DEFINE_ART_SERVICE_INTERFACE_IMPL(arttest::LocalServiceTestInterfaceImpl, arttest::LocalServiceTestInterface)
