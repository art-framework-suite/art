#ifndef art_Utilities_GetEnvironmentVariable_h
#define art_Utilities_GetEnvironmentVariable_h

#include <cstdlib>
#include <string>

namespace art {
  inline
  std::string getEnvironmentVariable (std::string const& name, std::string const& defaultValue = std::string()) {
    char *p = ::getenv(name.c_str());
    return (p ? std::string(p) : defaultValue);
  }
}
#endif /* art_Utilities_GetEnvironmentVariable_h */

// Local Variables:
// mode: c++
// End:
