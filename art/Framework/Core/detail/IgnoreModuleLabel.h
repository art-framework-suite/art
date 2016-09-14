#ifndef art_Framework_Core_detail_IgnoreModuleLabel_h
#define art_Framework_Core_detail_IgnoreModuleLabel_h

#include <set>
#include <string>

namespace art {
  namespace detail {

    struct IgnoreModuleLabel {
      std::set<std::string> operator()()
      {
        return {"module_label"};
      }
    };

  }
}

#endif

// Local variables:
// mode: c++
// End:
