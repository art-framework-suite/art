#ifndef FWCore_MessageLogger_ExceptionMessages
#define FWCore_MessageLogger_ExceptionMessages

#include <exception>


namespace artZ {
  class Exception;
}

namespace art {

  void printCmsException(artZ::Exception& e, char const* prog = 0);
  void printBadAllocException(char const *prog = 0);
  void printStdException(std::exception& e, char const *prog = 0);
  void printUnknownException(char const *prog = 0);

}  // namespace art

#endif  // FWCore_MessageLogger_ExceptionMessages
