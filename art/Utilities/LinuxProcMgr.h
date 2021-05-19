#ifndef art_Utilities_LinuxProcMgr_h
#define art_Utilities_LinuxProcMgr_h

// ================================================================
// LinuxProcMgr
//
// Responsible for retrieving procfs information.
//
// A different implementation may be necessary to guarantee that we do
// not open too many file descriptors.
// ================================================================

#include "art/Utilities/LinuxProcData.h"

#include <sys/types.h>

#include <cstdio>
#include <string>

namespace art {

  class LinuxProcMgr {
  public:
    LinuxProcMgr() noexcept(false);
    ~LinuxProcMgr() noexcept;

    LinuxProcData::proc_tuple getCurrentData() const noexcept(false);
    double
    getVmPeak() const noexcept(false)
    {
      return getStatusData_("VmPeak");
    }
    double
    getVmHWM() const noexcept(false)
    {
      return getStatusData_("VmHWM");
    }

    LinuxProcMgr(LinuxProcMgr const&) = delete;
    LinuxProcMgr(LinuxProcMgr&&) = delete;
    LinuxProcMgr& operator=(LinuxProcMgr const&) = delete;
    LinuxProcMgr& operator=(LinuxProcMgr&&) = delete;

  private:
    double getStatusData_(std::string const& field) const noexcept(false);

    pid_t const pid_;
    long const pgSize_;
    FILE* const file_;
  };

} // namespace art

#endif /* art_Utilities_LinuxProcMgr_h */

// Local variables:
// mode:c++
// End:
