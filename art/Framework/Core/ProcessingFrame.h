#ifndef art_Framework_Core_ProcessingFrame_h
#define art_Framework_Core_ProcessingFrame_h

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/ScheduleID.h"

namespace art {
  class ProcessingFrame {
  public:
    explicit ProcessingFrame(ScheduleID const sid) : scheduleID_{sid} {}

    template <typename T>
    ServiceHandle<T>
    serviceHandle() const
    {
      return ServiceHandle<T>{};
    }

    auto
    scheduleID() const
    {
      return scheduleID_;
    }

  private:
    ScheduleID const scheduleID_;
  };
}

#endif /* art_Framework_Core_ProcessingFrame_h */

// Local Variables:
// mode: c++
// End:
