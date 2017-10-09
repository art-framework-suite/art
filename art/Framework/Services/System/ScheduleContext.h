#ifndef art_Framework_Services_System_ScheduleContext_h
#define art_Framework_Services_System_ScheduleContext_h

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Utilities/ScheduleID.h"

#include <atomic>

namespace art {
  class ScheduleContext;

  namespace detail {
    class ScheduleContextSetter; // Forward declaration for friendship.
  }
}

extern int main(); // Forward declaration for friendship of test.

class art::ScheduleContext {
public:
  explicit ScheduleContext() noexcept = default;
  ScheduleID currentScheduleID() const;

  friend class detail::ScheduleContextSetter;
  friend int ::main();

private:
  bool setContext();
  bool resetContext();

  using flag_type = unsigned char;
  std::atomic<flag_type> in_context_{false};
};

inline bool
art::ScheduleContext::setContext()
{
  return in_context_.fetch_or(true);
}

inline bool
art::ScheduleContext::resetContext()
{
  return in_context_.fetch_and(false);
}

DECLARE_ART_SYSTEM_SERVICE(art::ScheduleContext, GLOBAL)
#endif /* art_Framework_Services_System_ScheduleContext_h */

// Local Variables:
// mode: c++
// End:
