#ifndef art_Utilities_CPCSentry_h
#define art_Utilities_CPCSentry_h
// vim: set sw=2 expandtab :

#include "art/Utilities/CurrentProcessingContext.h"

namespace art {
  namespace detail {

    class CPCSentry {

    public: // MEMBER FUNCTIONS -- Special Member Functions
      ~CPCSentry();

      CPCSentry(CurrentProcessingContext const&);

      CPCSentry(CPCSentry const&) = delete;

      CPCSentry(CPCSentry&&) = delete;

      CPCSentry& operator=(CPCSentry const&) = delete;

      CPCSentry& operator=(CPCSentry&&) = delete;

    private: // MEMBER DATA
      CurrentProcessingContext cpc_;
    };

  } // namespace detail
} // namespace art

#endif /* art_Utilities_CPCSentry_h */

// Local Variables:
// mode: c++
// End:
