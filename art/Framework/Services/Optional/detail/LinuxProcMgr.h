#ifndef art_Framework_Services_Optional_detail_LinuxProcMgr_h
#define art_Framework_Services_Optional_detail_LinuxProcMgr_h

#include "art/Framework/Services/Optional/detail/LinuxProcData.h"

namespace art {
  namespace detail {

    class LinuxProcMgr {
    public:

      LinuxProcMgr();
      ~LinuxProcMgr();

      LinuxProcData::proc_array getCurrentData() const;
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
      int fd_ {};
    };

  }
}

#endif /* art_Framework_Services_Optional_detail_LinuxProcMgr_h */

// Local variables:
// mode:c++
// End:
