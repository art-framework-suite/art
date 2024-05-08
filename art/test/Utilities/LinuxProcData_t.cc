#include "art/Utilities/LinuxProcData.h"
#include <tuple>

struct proc_tuple{
  //fake proc_tuple
};

template <typename T>
concept enforcement = requires (art::LinuxProcData l, art::LinuxProcData::proc_tuple p) {
  {l.getValueInMB<T>(p)};
};

int main() {
  auto proc_data = art::LinuxProcData::make_proc_tuple(5000000000, 2000000000);
  [[maybe_unused]] double vsize_in_mb = art::LinuxProcData::getValueInMB<art::LinuxProcData::vsize_t>(proc_data);
  [[maybe_unused]] double rss_in_mb = art::LinuxProcData::getValueInMB<art::LinuxProcData::rss_t>(proc_data);
  static_assert(enforcement<int>);
}
