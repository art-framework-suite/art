#ifndef art_test_Integration_Wanted_h
#define art_test_Integration_Wanted_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  namespace test {

    class Wanted {

    public:
      explicit Wanted(fhicl::ParameterSet const&) {}

      int
      getCachedValue() const
      {
        return cached_value_;
      }

      void
      setValue(int const value)
      {
        cached_value_ = value;
      }

    private:
      int cached_value_;
    };

  } // namespace test
} // namespace art

DECLARE_ART_SERVICE(art::test::Wanted, LEGACY)

#endif /* art_test_Integration_Wanted_h */

// Local Variables:
// mode: c++
// End:
