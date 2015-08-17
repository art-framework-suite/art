#ifndef art_Framework_Services_Optional_SimpleMemoryCheckDarwin_h
#define art_Framework_Services_Optional_SimpleMemoryCheckDarwin_h

// ======================================================================
//
// SimpleMemoryCheckDarwin
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceTable.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace art {

  class ActivityRegistry;

  class SimpleMemoryCheck {
  public:

    struct Config{};
    using Parameters = ServiceTable<Config>;
    SimpleMemoryCheck(Parameters const &, ActivityRegistry &) {
      mf::LogAbsolute("SimpleMemoryCheck") << "\n"
                                           << "Service currently not supported for this operating system.\n"
                                           << "If desired, please log an issue with:\n\n"
                                           << "https://cdcvs.fnal.gov/redmine/projects/cet-is/issues/new\n\n";
    }

  };

}  // art

#endif /* art_Framework_Services_Optional_SimpleMemoryCheckDarwin_h */

// Local variables:
// mode: c++
// End:
