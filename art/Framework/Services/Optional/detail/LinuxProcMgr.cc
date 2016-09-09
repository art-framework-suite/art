// ============================================================
//
// LinuxProcInfo
//
// ============================================================

#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "canvas/Utilities/Exception.h"

#include <regex>
#include <sstream>

extern "C" {
#include <fcntl.h>
#include <unistd.h>
}

namespace {

  // helper to ignore tokens in an istream
  struct token_ignore {
    explicit token_ignore(const unsigned ntokens) : ntokens_{ntokens} {}
    unsigned ntokens_;
  };

  std::istream& operator>>(std::istream& is, token_ignore const&& ig)
  {
    std::string tmp;
    unsigned i {};
    while (i++ < ig.ntokens_) is >> tmp;
    return is;
  }

} // anon. namespace

namespace art {
  namespace detail {

    //=======================================================
    LinuxProcMgr::LinuxProcMgr()
      : pid_{getpid()}
      , pgSize_{sysconf(_SC_PAGESIZE)}
    {
      std::ostringstream ost;
      ost << "/proc/" << pid_ << "/stat";
      if ((fd_ = open(ost.str().c_str(), O_RDONLY)) < 0) {
        throw Exception{errors::Configuration}
          << " Failed to open: " << ost.str() << std::endl;
      }
    }

    //=======================================================
    LinuxProcMgr::~LinuxProcMgr()
    {
      close(fd_);
    }

    //=======================================================
    LinuxProcData::proc_array LinuxProcMgr::getCurrentData() const
    {
      lseek(fd_, 0, SEEK_SET);

      char buf[400];
      int const cnt = read(fd_, buf, sizeof(buf));

      LinuxProcData::proc_array data;

      if (cnt < 0) {
        perror("Read of Proc file failed:");
      }
      else if (cnt > 0) {
        buf[cnt] = '\0';

        LinuxProcData::vsize_t vsize;
        LinuxProcData::rss_t   rss;

        std::istringstream iss {buf};
        iss >> token_ignore(22) >> vsize >> rss;

        data = {vsize/LinuxProcData::MB,
                rss*pgSize_/LinuxProcData::MB};
      }
      return data;

    }

    //=======================================================
    double LinuxProcMgr::getStatusData_(std::string const& field) const
    {
      std::ostringstream ost;
      ost << "cat /proc/" << pid_ << "/status";

      FILE* file = popen(ost.str().c_str(), "r");
      if (file == nullptr) {
        throw Exception{errors::Configuration}
          << " Failed to open: " << ost.str() << std::endl;
      }

      double value {};
      std::regex const pattern {"^"+field+R"(:\s*(\d+)\s*kB)"};
      while (!feof(file)) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), file) != nullptr) {
          std::cmatch cm;
          if (std::regex_search(buffer, cm, pattern)) {
            value = std::stod(cm.str(1))/LinuxProcData::kB; // convert to MB
            break;
          }
        }
      }
      return value;
    }

  } // namespace detail
} // namespace art
