#ifndef art_Utilities_LinuxProcMgr_h
#define art_Utilities_LinuxProcMgr_h

// ================================================================
// LinuxProcMgr
//
// Responsible for retrieving procfs information.
//
// MT-TODO: Once we decide to allow multiple modules to process the
// same event concurrently, we'll need to adjust how memory
// information is retrieved--it may need to be per-thread instead of
// per-schedule.
// ================================================================

#include "art/Utilities/LinuxProcData.h"
#include "art/Utilities/ScheduleID.h"

#include <vector>

namespace art {

  class LinuxProcMgr {
  public:
    using sid_size_type = ScheduleID::size_type;
    explicit LinuxProcMgr(sid_size_type nSchedules);
    ~LinuxProcMgr() noexcept;

    LinuxProcData::proc_tuple getCurrentData(sid_size_type) const;
    double
    getVmPeak() const
    {
      return getStatusData_("VmPeak");
    }
    double
    getVmHWM() const
    {
      return getStatusData_("VmHWM");
    }

    // Disable copy/move
    LinuxProcMgr(LinuxProcMgr const&) = delete;
    LinuxProcMgr(LinuxProcMgr&&) = delete;
    LinuxProcMgr& operator=(LinuxProcMgr const&) = delete;
    LinuxProcMgr& operator=(LinuxProcMgr&&) = delete;

  private:
    double getStatusData_(std::string const& field) const;

    pid_t pid_;
    long pgSize_;
    std::vector<int> fileDescriptors_{};
  };

} // namespace art

#endif /* art_Utilities_LinuxProcMgr_h */

// Local variables:
// mode:c++
// End:
