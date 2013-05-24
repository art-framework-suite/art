#ifndef art_Framework_Services_System_ScheduleContext_h
#define art_Framework_Services_System_ScheduleContext_h

#include "art/Utilities/ScheduleID.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"

#include <atomic>

namespace art {
  class ScheduleContext;

  namespace detail {
    class ScheduleContextSetter; // Forward declaration for friendship.
  }
}

extern int main(); //Forward declaration for friendship of test.

class art::ScheduleContext {
public:
  ScheduleContext();
  ScheduleID currentScheduleID();

  friend class detail::ScheduleContextSetter;
  friend int ::main();

private:
  bool setContext();
  bool resetContext();

  typedef unsigned char flag_type;
  std::atomic<flag_type> in_context_;
};

#ifndef __GCCXML__
inline
bool
art::ScheduleContext::
setContext()
{
  return in_context_.fetch_or(true);
}

inline
bool
art::ScheduleContext::
resetContext()
{
  return in_context_.fetch_and(false);
}
#endif

DECLARE_ART_SYSTEM_SERVICE(art::ScheduleContext, GLOBAL)
#endif /* art_Framework_Services_System_ScheduleContext_h */

// Local Variables:
// mode: c++
// End:
