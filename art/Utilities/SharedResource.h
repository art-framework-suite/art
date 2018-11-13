#ifndef art_Framework_Core_SharedResource_h
#define art_Framework_Core_SharedResource_h

#include <string>

namespace art {
  namespace detail {
    struct SharedResource_t {
      SharedResource_t(std::string const& name, bool demangle);
      std::string name;
    };
  }

  template <typename T>
  detail::SharedResource_t SharedResource{typeid(T).name(), true};
}

#endif
