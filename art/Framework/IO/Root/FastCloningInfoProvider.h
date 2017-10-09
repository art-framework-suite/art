#ifndef art_Framework_IO_Root_FastCloningInfoProvider_h
#define art_Framework_IO_Root_FastCloningInfoProvider_h

#include "cetlib/exempt_ptr.h"
#include <unistd.h>

namespace art {
  class FastCloningInfoProvider;
  class DecrepitRelicInputSourceImplementation;
} // namespace art

class art::FastCloningInfoProvider {
public:
  FastCloningInfoProvider() : input_() {}
  explicit FastCloningInfoProvider(
    cet::exempt_ptr<DecrepitRelicInputSourceImplementation> input);

  bool fastCloningPermitted() const;

  off_t remainingEvents() const;
  off_t remainingSubRuns() const;

private:
  cet::exempt_ptr<DecrepitRelicInputSourceImplementation> input_;
};

inline bool
art::FastCloningInfoProvider::fastCloningPermitted() const
{
  return !input_.empty();
}

#endif /* art_Framework_IO_Root_FastCloningInfoProvider_h */

// Local Variables:
// mode: c++
// End:
