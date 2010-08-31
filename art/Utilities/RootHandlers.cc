
#include "art/Utilities/RootHandlers.h"

namespace edm
{
  RootHandlers::RootHandlers() { }

  RootHandlers::~RootHandlers() { }

  void RootHandlers::disableErrorHandler() {disableErrorHandler_();}
  void RootHandlers::enableErrorHandler() {enableErrorHandler_();}

}
