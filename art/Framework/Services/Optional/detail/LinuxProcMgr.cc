// ============================================================
//
// LinuxProcInfo
//
// ============================================================

#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "art/Utilities/Exception.h"

#include <sstream>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

namespace {

  // helper to ignore tokens in an istream

  struct token_ignore{
    explicit token_ignore(const unsigned ntokens) : ntokens_(ntokens) {}
    unsigned ntokens_;
  };

  std::istream& operator>>( std::istream& is, const token_ignore&& ig) {
    std::string tmp;
    unsigned i(0);
    while ( i++ < ig.ntokens_ ) is >> tmp;
    return is;
  }

} // anon. namespace

namespace art {
  namespace detail {

    //=======================================================
    LinuxProcMgr::LinuxProcMgr()
      : fd_() 
      , pgSize_(sysconf(_SC_PAGESIZE))
    {

      std::ostringstream ost;
      ost << "/proc/" << getpid() << "/stat";
      if ((fd_ = open(ost.str().c_str(), O_RDONLY)) < 0) {
        throw art::Exception(errors::Configuration)
          << " Failed to open: " << ost.str() << std::endl;
      }

    }

    //=======================================================
    LinuxProcMgr::~LinuxProcMgr()
    {
      close(fd_);
    }

    //=======================================================
    LinuxProcData::proc_array LinuxProcMgr::getCurrentData()
    {
      
      lseek(fd_, 0, SEEK_SET);
      
      char buf[400];
      int const cnt = read(fd_, buf, sizeof(buf));
      
      LinuxProcData::proc_array data;
      
      if ( cnt < 0 ) {
        perror("Read of Proc file failed:");
      }
      else if (cnt > 0) {
        buf[cnt] = '\0';
        
        LinuxProcData::vsize_t vsize;
        LinuxProcData::rss_t   rss;
        
        std::istringstream iss( buf );
        iss >> token_ignore(22) >> vsize >> rss;
        
        data = { vsize/LinuxProcData::MB, 
                 rss*pgSize_/LinuxProcData::MB };
        
      }
      return data;

    }

  } // namespace detail
} // namespace art

