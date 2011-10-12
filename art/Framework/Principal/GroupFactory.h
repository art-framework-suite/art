#ifndef art_Framework_Principal_GroupFactory_h
#define art_Framework_Principal_GroupFactory_h
// A collection of functions to generate Groups.

#include "art/Framework/Principal/fwd.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/memory"

namespace art {
  class BranchDescription;
  class EDProduct;
  class ProductID;

  namespace gfactory {
    std::auto_ptr<Group>
    make_group(BranchDescription const & bd,
               ProductID const & pid);
    std::auto_ptr<Group>
    make_group(BranchDescription const & bd,
               ProductID const & pid,
               cet::exempt_ptr<Worker> productProducer,
               cet::exempt_ptr<EventPrincipal> onDemandPrincipal);
    std::auto_ptr<Group>
    make_group(std::auto_ptr<EDProduct> edp,
               BranchDescription const & bd,
               ProductID const & pid);
  }
}
#endif /* art_Framework_Principal_GroupFactory_h */

// Local Variables:
// mode: c++
// End:
