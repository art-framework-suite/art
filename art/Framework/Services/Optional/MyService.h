#ifndef art_Framework_Services_System_MyService_h
#define art_Framework_Services_System_MyService_h

// ======================================================================
//
// MyService
//
// This service is mine.
//
// ======================================================================

#include "art/Framework/Services/Optional/MyServiceInterface.h"

namespace art {
  class ActivityRegistry;
  class MyService;
}

namespace fhicl {
  class ParameterSet;
}

// ----------------------------------------------------------------------

class art::MyService : public art::MyServiceInterface
{
public:
  MyService(fhicl::ParameterSet const&, art::ActivityRegistry&);

  // Use compiler-generated copy c'tor, copy assignment, and d'tor

private:

};  // MyService

// ======================================================================

#endif /* art_Framework_Services_System_MyService_h */

// Local Variables:
// mode: c++
// End:
