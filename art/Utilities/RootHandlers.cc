
#include "art/Utilities/RootHandlers.h"

namespace art
{
  RootHandlers::RootHandlers() { }

  RootHandlers::~RootHandlers() { }

  void RootHandlers::disableErrorHandler() {disableErrorHandler_();}
  void RootHandlers::enableErrorHandler() {enableErrorHandler_();}

}
