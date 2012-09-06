#ifndef test_Framework_Services_System_MySystemService_h
#define test_Framework_Services_System_MySystemService_h

// ======================================================================
//
// MySystemService
//
// This service is mine.
//
// ======================================================================

namespace arttest {
  class MySystemService;
}

namespace art {
  class ActivityRegistry;
}

namespace fhicl {
  class ParameterSet;
}

#include "test/Framework/Services/Interfaces/MySystemServiceInterface.h"

// ----------------------------------------------------------------------

class arttest::MySystemService : public arttest::MySystemServiceInterface
{
public:
  MySystemService(fhicl::ParameterSet const&, art::ActivityRegistry&);

  // Use compiler-generated copy c'tor, copy assignment, and d'tor

private:

};  // MySystemService

// ======================================================================

#endif /* test_Framework_Services_System_MySystemService_h */

// Local Variables:
// mode: c++
// End:
