#ifndef art_Utilities_SharedResource_h
#define art_Utilities_SharedResource_h

#include <string>
#include <typeinfo>

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

#endif /* art_Utilities_SharedResource_h */

// Local Variables:
// mode: c++
// End:
