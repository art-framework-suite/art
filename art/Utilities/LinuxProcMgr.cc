// ============================================================
//
// LinuxProcInfo
//
// ============================================================

#include "art/Utilities/LinuxProcMgr.h"
#include "art/Utilities/LinuxProcData.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/assert_only_one_thread.h"

#include <regex>
#include <sstream>

extern "C" {
#include <unistd.h>
}

namespace {

  // helper to ignore tokens in an istream
  struct token_ignore {
    explicit token_ignore(const unsigned ntokens) : ntokens_{ntokens} {}
    unsigned ntokens_;
  };

  std::istream&
  operator>>(std::istream& is, token_ignore const&& ig)
  {
    std::string tmp;
    unsigned i{};
    while (i++ < ig.ntokens_)
      is >> tmp;
    return is;
  }

} // namespace

namespace art {

  //=======================================================
  LinuxProcMgr::LinuxProcMgr(sid_size_type const nSchedules)
    : pid_{getpid()}, pgSize_{sysconf(_SC_PAGESIZE)}
  {
    std::ostringstream ost;
    ost << "/proc/" << pid_ << "/stat";

    for (sid_size_type i{}; i < nSchedules; ++i) {
      auto file = fopen(ost.str().c_str(), "r");
      if (file == nullptr) {
        throw Exception{errors::Configuration}
          << " Failed to open: " << ost.str() << " for schedule: " << i << '\n';
      }
      files_.push_back(file);
    }
  }

  //=======================================================
  LinuxProcMgr::~LinuxProcMgr() noexcept
  {
    for (auto const file : files_) {
      fclose(file);
    }
  }

  //=======================================================
  LinuxProcData::proc_tuple
  LinuxProcMgr::getCurrentData(sid_size_type const sid) const
  {
    auto file = files_[sid];

    int const seek_result{fseek(file, 0, SEEK_SET)};
    if (seek_result != 0) {
      throw Exception{errors::FileReadError,
                      "Error while retrieving Linux proc data."}
        << "\nCould not reset position indicator while retrieving proc "
           "stat information.\n";
    }

    char buf[400];
    size_t const cnt{fread(buf, 1, sizeof(buf), file)};

    auto data = LinuxProcData::make_proc_tuple();

    if (cnt == 0) {
      throw Exception{errors::FileReadError,
                      "Error while retrieving Linux proc data."}
        << "\nCould not read proc stat information.\n";
    }

    buf[cnt] = '\0';

    LinuxProcData::vsize_t::value_type vsize;
    LinuxProcData::rss_t::value_type rss;

    std::istringstream iss{buf};
    iss >> token_ignore(22) >> vsize >> rss;

    data = LinuxProcData::make_proc_tuple(vsize, rss * pgSize_);
    return data;
  }

  //=======================================================
  double
  LinuxProcMgr::getStatusData_(std::string const& field) const
  {
    CET_ASSERT_ONLY_ONE_THREAD();

    std::ostringstream ost;
    ost << "/proc/" << pid_ << "/status";

    auto file = fopen(ost.str().c_str(), "r");
    if (file == nullptr) {
      throw Exception{errors::Configuration} << " Failed to open: " << ost.str()
                                             << std::endl;
    }

    double value{};
    std::regex const pattern{"^" + field + R"(:\s*(\d+)\s*kB)"};
    while (!feof(file)) {
      char buffer[128];
      if (fgets(buffer, sizeof(buffer), file) != nullptr) {
        std::cmatch cm;
        if (std::regex_search(buffer, cm, pattern)) {
          // Reported value from proc (although labeled 'kB') is
          // actually in KiB.  Will convert to base-10 MB.
          value = std::stod(cm.str(1)) * LinuxProcData::KiB / LinuxProcData::MB;
          break;
        }
      }
    }
    fclose(file);
    return value;
  }

} // namespace art
