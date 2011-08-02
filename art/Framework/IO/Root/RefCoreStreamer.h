#ifndef art_Framework_IO_Root_RefCoreStreamer_h
#define art_Framework_IO_Root_RefCoreStreamer_h

#include "art/Framework/Principal/fwd.h"
#include "cetlib/exempt_ptr.h"

#include "TClassStreamer.h"
#include "TClassRef.h"

class TBuffer;

namespace art {
  class RefCoreStreamer;

  void
  configureRefCoreStreamer(cet::exempt_ptr<EventPrincipal const> groupFinder =
                           cet::exempt_ptr<EventPrincipal const>());
}

class art::RefCoreStreamer : public TClassStreamer {
public:
  explicit RefCoreStreamer(cet::exempt_ptr<EventPrincipal const> groupFinder =
                           cet::exempt_ptr<EventPrincipal const>())
    :
    groupFinder_(groupFinder)
  {}

  void setGroupFinder(cet::exempt_ptr<EventPrincipal const> groupFinder) {
    groupFinder_ = groupFinder;
  }
  void operator() (TBuffer &R_b, void *objp);

private:
  cet::exempt_ptr<EventPrincipal const> groupFinder_;
};



#endif /* art_Framework_IO_Root_RefCoreStreamer_h */

// Local Variables:
// mode: c++
// End:
