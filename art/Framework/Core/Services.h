#ifndef art_Framework_Core_Services_h
#define art_Framework_Core_Services_h

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/ScheduleID.h"

namespace art {
  class Services {
  public:
    explicit Services(ScheduleID const sid) : scheduleID_{sid} {}

    template <typename T>
    T const&
    get() const
    {
      return *ServiceHandle<T const>{};
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

#endif /* art_Framework_Core_Services_h */

// Local Variables:
// mode: c++
// End:
