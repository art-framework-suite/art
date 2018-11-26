#ifndef art_Utilities_RootHandlers_h
#define art_Utilities_RootHandlers_h

// ----------------------------------------------------------------------

#include "art/Utilities/fwd.h"

namespace art {

  class RootHandlers {
  public:
    RootHandlers();
    virtual ~RootHandlers();
    void disableErrorHandler();
    void enableErrorHandler();

  private:
    virtual void disableErrorHandler_() = 0;
    virtual void enableErrorHandler_() = 0;
  }; // RootHandlers

} // art

// ======================================================================

#endif /* art_Utilities_RootHandlers_h */

// Local Variables:
// mode: c++
// End:
