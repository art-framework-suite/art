#ifndef art_Framework_Core_detail_LegacyModule_h
#define art_Framework_Core_detail_LegacyModule_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/detail/SharedModule.h"
#include "art/Utilities/ScheduleID.h"

namespace art {
  namespace detail {

    class LegacyModule : public SharedModule {
    public:
      auto
      scheduleID() const
      {
        return scheduleID_;
      }

      void
      setScheduleID(ScheduleID const sid) noexcept
      {
        scheduleID_ = sid;
      }

      class ScheduleIDSentry;

    private:
      ScheduleID scheduleID_;
    };

    class LegacyModule::ScheduleIDSentry {
    public:
      ScheduleIDSentry(LegacyModule& mod, ScheduleID const sid) noexcept
        : mod_{mod}
      {
        mod_.setScheduleID(sid);
      }

      ~ScheduleIDSentry() noexcept { mod_.setScheduleID(ScheduleID{}); }

    private:
      LegacyModule& mod_;
    };
  }
}

  // Local Variables:
  // mode: c++
  // End:

#endif /* art_Framework_Core_detail_LegacyModule_h */
