#ifndef art_Framework_IO_Root_detail_orderedProcessNames_h
#define art_Framework_IO_Root_detail_orderedProcessNames_h

#include <string>
#include <vector>

namespace art {
  namespace detail {

    std::vector<std::string> orderedProcessNames(std::string const& currentProcessName);

  }
}

#endif

// Local variables:
// mode: c++
// End:
