#ifndef test_Integration_Wanted_h
#define test_Integration_Wanted_h
// Service with a name near the end of the alphabet with a hook to make
// sure it gets called.

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

namespace arttest {
  class Wanted;
}

class arttest::Wanted {
public:
  Wanted(fhicl::ParameterSet const &, art::ActivityRegistry &);

  bool postBeginJobCalled() const { return postBeginJobCalled_; }

private:
  void postBeginJob();

  bool postBeginJobCalled_;
};
DECLARE_ART_SERVICE(arttest::Wanted, LEGACY)
#endif /* test_Integration_Wanted_h */

// Local Variables:
// mode: c++
// End:
