#ifndef art_Utilities_GetPassID_h
#define art_Utilities_GetPassID_h

#include <string>

namespace art {
  inline
  std::string getPassID () {
    static std::string passID;
    // return empty string for now.
    return passID;
  }
}
#endif /* art_Utilities_GetPassID_h */

// Local Variables:
// mode: c++
// End:
