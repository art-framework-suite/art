#ifndef art_Utilities_ExceptionMessages_h
#define art_Utilities_ExceptionMessages_h

#include "cetlib_except/exception.h"

namespace art {
  void printArtException(cet::exception const& e, char const* prog = nullptr);
  void printBadAllocException(char const* prog = nullptr);
  void printStdException(std::exception const& e, char const* prog = nullptr);
  void printUnknownException(char const* prog = nullptr);
}

#endif /* art_Utilities_ExceptionMessages_h */

// Local Variables:
// mode: c++
// End:
