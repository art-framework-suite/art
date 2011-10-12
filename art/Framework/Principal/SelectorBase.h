#ifndef art_Framework_Principal_SelectorBase_h
#define art_Framework_Principal_SelectorBase_h

/*----------------------------------------------------------------------

Selector: Base class for all "selector" objects, used to select
EDProducts based on information in the associated Provenance.

Developers who make their own Selectors should inherit from SelectorBase.

----------------------------------------------------------------------*/

#include "art/Framework/Principal/fwd.h"

namespace art {
  class BranchDescription;
}

class art::SelectorBase {
public:
  virtual ~SelectorBase();
  bool match(BranchDescription const & p) const { return doMatch(p); }
  virtual SelectorBase * clone() const = 0;
private:
  virtual bool doMatch(BranchDescription const & p) const = 0;
};

#endif /* art_Framework_Principal_SelectorBase_h */

// Local Variables:
// mode: c++
// End:
