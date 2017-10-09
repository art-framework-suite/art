#ifndef art_test_Framework_Services_Optional_PSTestInterfaceImpl_h
#define art_test_Framework_Services_Optional_PSTestInterfaceImpl_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/ScheduleID.h"
#include "art/test/Framework/Services/Interfaces/PSTestInterface.h"
#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class PSTestInterfaceImpl;
}

class arttest::PSTestInterfaceImpl : public arttest::PSTestInterface {
public:
  PSTestInterfaceImpl(fhicl::ParameterSet const&,
                      art::ActivityRegistry&,
                      art::ScheduleID);

  art::ScheduleID
  schedule() const override
  {
    return sID_;
  }

private:
  art::ScheduleID sID_;
};

// Service declaration.
DECLARE_ART_SERVICE_INTERFACE_IMPL(arttest::PSTestInterfaceImpl,
                                   arttest::PSTestInterface,
                                   PER_SCHEDULE)
#endif /* art_test_Framework_Services_Optional_PSTestInterfaceImpl_h */

// Local Variables:
// mode: c++
// End:
