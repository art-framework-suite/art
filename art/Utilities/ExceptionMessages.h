#ifndef FWCore_MessageLogger_ExceptionMessages
#define FWCore_MessageLogger_ExceptionMessages

#include "cetlib/exception.h"

// ----------------------------------------------------------------------

namespace art {

  void printCmsException(cet::exception& e, char const* prog = 0);
  void printBadAllocException(char const *prog = 0);
  void printStdException(std::exception& e, char const *prog = 0);
  void printUnknownException(char const *prog = 0);

}  // art

// ======================================================================

#endif
