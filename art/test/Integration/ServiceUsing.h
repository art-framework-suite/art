#ifndef art_test_Integration_ServiceUsing_h
#define art_test_Integration_ServiceUsing_h

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/test/Integration/Wanted.h"

namespace art {
  namespace test {
    class ServiceUsing;
  }
}

class art::test::ServiceUsing {
public:
  ServiceUsing(fhicl::ParameterSet const&, art::ActivityRegistry&);

  int getCachedValue() const { return cached_value_; }
  bool postBeginJobCalled() const { return postBeginJobCalled_; }

private:

  void postBeginJob();
  bool postBeginJobCalled_ {false};
  int cached_value_ {};
  ServiceHandle<Wanted> wanted_ {};
};

DECLARE_ART_SERVICE(art::test::ServiceUsing, LEGACY)
#endif /* art_test_Integration_ServiceUsing_h */

// Local Variables:
// mode: c++
// End:
