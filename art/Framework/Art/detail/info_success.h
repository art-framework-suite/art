#ifndef art_Framework_Art_detail_info_success_h
#define art_Framework_Art_detail_info_success_h

#include <limits>

namespace art {
  namespace detail {
    constexpr int info_success()
    {
      return std::numeric_limits<int>::max();
    }
  }
}

#endif
