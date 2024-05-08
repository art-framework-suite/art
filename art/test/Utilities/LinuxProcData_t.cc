#include "art/Utilities/LinuxProcData.h"
#include <tuple>

template <typename T>
concept enforcement =
  requires(art::LinuxProcData l, art::LinuxProcData::proc_tuple p) {
    {
      l.getValueInMB<T>(p)
    };
  };

int
main()
{
  static_assert(enforcement<art::LinuxProcData::rss_t>);
  static_assert(enforcement<art::LinuxProcData::vsize_t>);
  static_assert(enforcement<int>);
}
