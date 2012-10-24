#ifndef test_Framework_Services_Optional_PSTestInterfaceImpl_h
#define test_Framework_Services_Optional_PSTestInterfaceImpl_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/ScheduleID.h"
#include "fhiclcpp/ParameterSet.h"
#include "test/Framework/Services/Interfaces/PSTestInterface.h"

namespace arttest {
  class PSTestInterfaceImpl;
}

class arttest::PSTestInterfaceImpl : public arttest::PSTestInterface {
public:
  PSTestInterfaceImpl(fhicl::ParameterSet const &,
                      art::ActivityRegistry &,
                      art::ScheduleID);

  art::ScheduleID schedule() const override { return sID_; }
private:
  art::ScheduleID sID_;
};

// Service declaration.
DECLARE_ART_SERVICE_INTERFACE_IMPL(arttest::PSTestInterfaceImpl, arttest::PSTestInterface, PER_SCHEDULE)
#endif /* test_Framework_Services_Optional_PSTestInterfaceImpl_h */

// Local Variables:
// mode: c++
// End:
