#ifndef art_Framework_Principal_UnscheduledHandler_h
#define art_Framework_Principal_UnscheduledHandler_h

// ======================================================================
//
// UnscheduledHandler - Interface to allow handling unscheduled processing
//
// This class is used internally to the Framework for running the
// unscheduled case.  It is written as a base class to keep the
// EventPrincipal class from having too much physical coupling with the
// implementation.
//
// ======================================================================

#include "art/Framework/Principal/fwd.h"
#include <string>

namespace art {

  class UnscheduledHandler
  {
  public:
  UnscheduledHandler() {}
    virtual ~UnscheduledHandler() {}

    //returns true if found an EDProducer and ran it
    bool tryToFill(std::string const& label,
                   EventPrincipal& iEvent) {
      return tryToFillImpl(label, iEvent);
    }

  private:
    UnscheduledHandler(UnscheduledHandler const&); // stop default

    const UnscheduledHandler& operator=(UnscheduledHandler const&); // stop default

    virtual bool tryToFillImpl(std::string const&,
                                EventPrincipal&) = 0;
  };  // UnscheduledHandler

}  // art

// ======================================================================

#endif /* art_Framework_Principal_UnscheduledHandler_h */

// Local Variables:
// mode: c++
// End:
