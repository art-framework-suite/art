#ifndef art_Framework_IO_Root_detail_DummyProductCache_h
#define art_Framework_IO_Root_detail_DummyProductCache_h

#include "canvas/Persistency/Common/EDProduct.h"

#include <map>
#include <memory>
#include <string>

namespace art {
  namespace detail {

    class DummyProductCache {
    public:
      EDProduct const* product(std::string const& wrappedName);

    private:
      std::map<std::string, std::unique_ptr<EDProduct>> dummies_;
    };
  }
}

#endif /* art_Framework_IO_Root_detail_DummyProductCache_h */

// Local variables:
// mode: c++
// End:
