#ifndef test_Framework_Services_Optional_LocalServiceTest_h
#define test_Framework_Services_Optional_LocalServiceTest_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/ScheduleID.h"

#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class LocalServiceTest;
}

class arttest::LocalServiceTest {
public:
  LocalServiceTest(art::ScheduleID); // For system service tests.

  LocalServiceTest(fhicl::ParameterSet const &,
         art::ActivityRegistry &,
         art::ScheduleID);

  art::ScheduleID schedule() const { return sID_; }
private:
  art::ScheduleID sID_;
};

// Service declaration.
DECLARE_ART_SERVICE(arttest::LocalServiceTest, LOCAL)
#endif /* test_Framework_Services_Optional_LocalServiceTest_h */

// Local Variables:
// mode: c++
// End:
