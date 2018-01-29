#ifndef art_Utilities_ExceptionMessages_h
#define art_Utilities_ExceptionMessages_h

#include "cetlib_except/exception.h"

// ----------------------------------------------------------------------

namespace art {

  void printArtException(cet::exception const& e, char const* prog = 0);
  void printBadAllocException(char const* prog = 0);
  void printStdException(std::exception const& e, char const* prog = 0);
  void printUnknownException(char const* prog = 0);

} // art

  // ======================================================================

#endif /* art_Utilities_ExceptionMessages_h */

// Local Variables:
// mode: c++
// End:
