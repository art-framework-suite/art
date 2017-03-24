// ============================================================
//
// LinuxProcInfo
//
// ============================================================

#include "art/Framework/Services/Optional/detail/LinuxProcData.h"
#include "art/Framework/Services/Optional/detail/LinuxProcMgr.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/assert_only_one_thread.h"

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
    LinuxProcMgr::LinuxProcMgr(sid_size_type const nSchedules)
      : pid_{getpid()}
      , pgSize_{sysconf(_SC_PAGESIZE)}
    {
      std::ostringstream ost;
      ost << "/proc/" << pid_ << "/stat";

      for (sid_size_type i {}; i < nSchedules; ++i) {
        auto fd = open(ost.str().c_str(), O_RDONLY);
        if (fd < 0) {
          throw Exception{errors::Configuration}
          << " Failed to open: " << ost.str() << " for schedule: " << i << '\n';
        }
        fileDescriptors_.push_back(fd);
      }
    }

    //=======================================================
    LinuxProcMgr::~LinuxProcMgr() noexcept
    {
      for (auto const fd : fileDescriptors_) {
        close(fd);
      }
    }

    //=======================================================
    LinuxProcData::proc_tuple
    LinuxProcMgr::getCurrentData(sid_size_type const sid) const
    {
      auto& fd = fileDescriptors_[sid];

      lseek(fd, 0, SEEK_SET);

      char buf[400];
      ssize_t const cnt {read(fd, buf, sizeof(buf))};

      auto data = LinuxProcData::make_proc_tuple();

      if (cnt < 0) {
        perror("Read of Proc file failed:");
      }
      else if (cnt > 0) {
        buf[cnt] = '\0';

        LinuxProcData::vsize_t::value_type vsize;
        LinuxProcData::rss_t::value_type rss;

        std::istringstream iss {buf};
        iss >> token_ignore(22) >> vsize >> rss;

        data = LinuxProcData::make_proc_tuple(vsize, rss*pgSize_);
      }
      return data;
    }

    //=======================================================
    double
    LinuxProcMgr::getStatusData_(std::string const& field) const
    {
      CET_ASSERT_ONLY_ONE_THREAD();

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
            // Reported value from proc (although labeled 'kB') is
            // actually in KiB.  Will convert to base-10 MB.
            value = std::stod(cm.str(1))*LinuxProcData::KiB/LinuxProcData::MB;
            break;
          }
        }
      }
      return value;
    }

  } // namespace detail
} // namespace art
