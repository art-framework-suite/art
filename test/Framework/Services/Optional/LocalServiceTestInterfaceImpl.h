#ifndef test_Framework_Services_Optional_LocalServiceTestInterfaceImpl_h
#define test_Framework_Services_Optional_LocalServiceTestInterfaceImpl_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/Framework/Services/Interfaces/LocalServiceTestInterface.h"

namespace arttest {
  class LocalServiceTestInterfaceImpl;
}

class arttest::LocalServiceTestInterfaceImpl : public arttest::LocalServiceTestInterface {
public:
  LocalServiceTestInterfaceImpl(fhicl::ParameterSet const &,
                      art::ActivityRegistry &,
                      art::ScheduleID);

  art::ScheduleID schedule() const override { return sID_; }
private:
  art::ScheduleID sID_;
};

// Service declaration.
DECLARE_ART_SERVICE_INTERFACE_IMPL(arttest::LocalServiceTestInterfaceImpl, arttest::LocalServiceTestInterface, LOCAL)
#endif /* test_Framework_Services_Optional_LocalServiceTestInterfaceImpl_h */

// Local Variables:
// mode: c++
// End:
