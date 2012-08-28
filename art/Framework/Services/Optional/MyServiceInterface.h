#ifndef art_Framework_Services_System_MyServiceInterface_h
#define art_Framework_Services_System_MyServiceInterface_h

// ======================================================================
//
// MyServiceInterface
//
// This service is mine.
//
// ======================================================================

namespace art {
  class ActivityRegistry;
  class MyServiceInterface;
}

namespace fhicl {
  class ParameterSet;
}

// ----------------------------------------------------------------------

class art::MyServiceInterface
{
public:
  MyServiceInterface();

  // Use compiler-generated copy c'tor, copy assignment, and d'tor

private:

};  // MyServiceInterface

// ======================================================================

#endif /* art_Framework_Services_System_MyServiceInterface_h */

// Local Variables:
// mode: c++
// End:
