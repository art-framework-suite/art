#ifndef FWCore_MessageLogger_ExceptionMessages
#define FWCore_MessageLogger_ExceptionMessages

#include <exception>


namespace cms {
  class Exception;
}

namespace edm {

  void printCmsException(cms::Exception& e, char const* prog = 0);
  void printBadAllocException(char const *prog = 0);
  void printStdException(std::exception& e, char const *prog = 0);
  void printUnknownException(char const *prog = 0);

}  // namespace edm

#endif  // FWCore_MessageLogger_ExceptionMessages
