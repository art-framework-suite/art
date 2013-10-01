#ifndef test_Framework_Services_Interfaces_LocalServiceTestInterface_h
#define test_Framework_Services_Interfaces_LocalServiceTestInterface_h

// LocalServiceTestInterface: interface from which a test service can inherit.

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/ScheduleID.h"

namespace arttest {
  class LocalServiceTestInterface {
public:
    virtual ~LocalServiceTestInterface() = default;
    virtual art::ScheduleID schedule() const = 0;
  };
}

DECLARE_ART_SERVICE_INTERFACE(arttest::LocalServiceTestInterface,LOCAL)
#endif /* test_Framework_Services_Interfaces_LocalServiceTestInterface_h */

// Local Variables:
// mode: c++
// End:
