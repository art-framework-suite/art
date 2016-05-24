#ifndef art_Framework_Services_Optional_detail_LinuxProcMgr_h
#define art_Framework_Services_Optional_detail_LinuxProcMgr_h

#include "art/Framework/Services/Optional/detail/LinuxProcData.h"

extern "C" {
#include <sys/types.h>
}

namespace art {
  namespace detail {

    class LinuxProcMgr {
    public:

      LinuxProcMgr();
      ~LinuxProcMgr();

      LinuxProcData::proc_array getCurrentData() const;
      double getVmPeak() const;

    private:
      pid_t pid_;
      int   fd_ {};
      long  pgSize_;
    };

  }
}
#endif /* art_Framework_Services_Optional_detail_LinuxProcMgr_h */

// Local variables:
// mode:c++
// End:
