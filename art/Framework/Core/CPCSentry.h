#ifndef art_Framework_Core_CPCSentry_h
#define art_Framework_Core_CPCSentry_h

#include "cetlib/exempt_ptr.h"

// ======================================================================
//
// class CPCSentry uses RAII to make sure that the
// CurrentProcessingContext pointer it is guarding is set to the right
// value, and cleared at the right time.
//
// ======================================================================

namespace art {

  class CurrentProcessingContext;

  namespace detail {

    class CPCSentry
    {
    public:
      CPCSentry(cet::exempt_ptr<CurrentProcessingContext const>& c,
                cet::exempt_ptr<CurrentProcessingContext const> value) :
        referenced_{&c}
      {
        c = value;
      }

      ~CPCSentry() { *referenced_ = nullptr; }

    private:
      cet::exempt_ptr<CurrentProcessingContext const>* referenced_;
    };  // CPCSentry

  }  // detail
}  // art

// ======================================================================

#endif /* art_Framework_Core_CPCSentry_h */

// Local Variables:
// mode: c++
// End:
