#ifndef art_Utilities_unique_filename_h
#define art_Utilities_unique_filename_h
#include <string>

namespace art {
  std::string unique_filename(std::string stem, std::string extension = ".root");
}

#endif /* art_Utilities_unique_filename_h */

// Local Variables:
// mode: c++
// End:
