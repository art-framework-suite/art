#include "art/Framework/IO/Root/RootHandlers.h"

namespace art {

  RootHandlers::RootHandlers() = default;
  RootHandlers::~RootHandlers() = default;

  void
  RootHandlers::disableErrorHandler()
  {
    disableErrorHandler_();
  }
  void
  RootHandlers::enableErrorHandler()
  {
    enableErrorHandler_();
  }

} // namespace art
