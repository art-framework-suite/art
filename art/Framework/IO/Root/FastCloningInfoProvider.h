#ifndef art_Framework_IO_Root_FastCloningInfoProvider_h
#define art_Framework_IO_Root_FastCloningInfoProvider_h

#include "cetlib/exempt_ptr.h"

namespace art {
  class FastCloningInfoProvider;
  class RootInput;
}

class art::FastCloningInfoProvider {
 public:
  explicit FastCloningInfoProvider(cet::exempt_ptr<RootInput> input);

  bool fastCloningPermitted() const;

  off_t remainingEvents() const;
  off_t remainingSubRuns() const;

 private:
  cet::exempt_ptr<RootInput> input_;
};

#endif /* art_Framework_IO_Root_FastCloningInfoProvider_h */

// Local Variables:
// mode: c++
// End:
