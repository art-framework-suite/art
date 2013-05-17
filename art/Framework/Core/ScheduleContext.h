#ifndef art_Utilities_ScheduleContext_h
#define art_Utilities_ScheduleContext_h

#include "art/Utilities/ScheduleID.h"

namespace art {
  class EventProcessor; // Forward declaration for friendship.
  class ScheduleContext;
}

extern int main(); // Forward declaration for friendship.

class art::ScheduleContext {
public:
  static ScheduleID currentScheduleID();

  friend class art::EventProcessor;
  friend int ::main(); // Testing only.

private:
  ScheduleContext();
  static ScheduleContext & instance();
  bool setContext();
  bool resetContext();

  bool in_context_;
};

#endif /* art_Utilities_ScheduleContext_h */

// Local Variables:
// mode: c++
// End:
