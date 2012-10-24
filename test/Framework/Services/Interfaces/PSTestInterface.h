#ifndef test_Framework_Services_Interfaces_PSTestInterface_h
#define test_Framework_Services_Interfaces_PSTestInterface_h

// PSTestInterface: interface from which a test service can inherit.

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/ScheduleID.h"

namespace arttest {
  class PSTestInterface {
public:
    virtual ~PSTestInterface() = default;
    virtual art::ScheduleID schedule() const = 0;
  };
}

DECLARE_ART_SERVICE_INTERFACE(arttest::PSTestInterface,PER_SCHEDULE)
#endif /* test_Framework_Services_Interfaces_PSTestInterface_h */

// Local Variables:
// mode: c++
// End:
