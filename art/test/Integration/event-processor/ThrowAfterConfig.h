#ifndef art_test_Integration_event_processor_ThrowAfterConfig_h
#define art_test_Integration_event_processor_ThrowAfterConfig_h

#include "fhiclcpp/types/Atom.h"
#include <limits>

namespace art {
  namespace test {
    struct ThrowAfterConfig {
      fhicl::Atom<bool> throwFromCtor {fhicl::Name("throwFromCtor"), false};
      fhicl::Atom<unsigned> throwAfter {
        fhicl::Name("throwAfter"),
          fhicl::Comment("The 'throwAfter' parameter specifies the number\n"
                         "of events after which an exception should be thrown.\n"
                         "If it is desired to not throw at all, then the parameter\n"
                         "should not be specified, or it should be assigned a\n"
                         "value of '+infinity'."),
          std::numeric_limits<unsigned>::max()};
    };
  }
}

#endif /* art_test_Integration_event_processor_ThrowAfterConfig_h */

// Local Variables:
// mode: c++
// End:
