#ifndef art_test_Framework_Services_Optional_PSTest_h
#define art_test_Framework_Services_Optional_PSTest_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/ScheduleID.h"

#include "fhiclcpp/ParameterSet.h"

namespace arttest {
  class PSTest;
}

class arttest::PSTest {
public:
  PSTest(art::ScheduleID); // For system service tests.

  PSTest(fhicl::ParameterSet const &,
         art::ActivityRegistry &,
         art::ScheduleID);

  art::ScheduleID schedule() const { return sID_; }
private:
  art::ScheduleID sID_;
};

// Service declaration.
DECLARE_ART_SERVICE(arttest::PSTest, PER_SCHEDULE)
#endif /* art_test_Framework_Services_Optional_PSTest_h */

// Local Variables:
// mode: c++
// End:
