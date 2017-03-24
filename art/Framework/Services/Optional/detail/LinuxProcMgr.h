#ifndef art_Framework_Services_Optional_detail_LinuxProcMgr_h
#define art_Framework_Services_Optional_detail_LinuxProcMgr_h

// ================================================================
// LinuxProcMgr
//
// Responsible for retrieving procfs information used by the
// MemoryTracker.
//
// MT-TODO: Once we decide to allow multiple modules to process the
// same event concurrently, we'll need to adjust how memory
// information is retrieved--it may need to be per-thread instead of
// per-schedule.
// ================================================================

#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Utilities/ScheduleID.h"

#include <vector>

namespace art {
  namespace detail {

    class LinuxProcMgr {
      using sid_size_type = ScheduleID::size_type;
    public:

      explicit LinuxProcMgr(sid_size_type nSchedules);
      ~LinuxProcMgr() noexcept;

      LinuxProcData::proc_tuple getCurrentData(sid_size_type) const;
      double getVmPeak() const { return getStatusData_("VmPeak"); }
      double getVmHWM() const { return getStatusData_("VmHWM"); }

      // Disable copy/move
      LinuxProcMgr(LinuxProcMgr const&) = delete;
      LinuxProcMgr(LinuxProcMgr&&) = delete;
      LinuxProcMgr& operator=(LinuxProcMgr const&) = delete;
      LinuxProcMgr& operator=(LinuxProcMgr&&) = delete;

    private:
      double getStatusData_(std::string const& field) const;

      pid_t pid_;
      long pgSize_;
      std::vector<int> fileDescriptors_ {};
    };

  }
}

#endif /* art_Framework_Services_Optional_detail_LinuxProcMgr_h */

// Local variables:
// mode:c++
// End:
