#ifndef art_Framework_Services_System_MySystemService_h
#define art_Framework_Services_System_MySystemService_h

// ======================================================================
//
// MySystemService
//
// This service is mine.
//
// ======================================================================

namespace art {
  class ActivityRegistry;
  class MySystemService;
}

namespace fhicl {
  class ParameterSet;
}

// ----------------------------------------------------------------------

class art::MySystemService
{
public:
  MySystemService(fhicl::ParameterSet const&, art::ActivityRegistry&);

  // Use compiler-generated copy c'tor, copy assignment, and d'tor

private:

};  // MySystemService

// ======================================================================

#endif /* art_Framework_Services_System_MySystemService_h */

// Local Variables:
// mode: c++
// End:
