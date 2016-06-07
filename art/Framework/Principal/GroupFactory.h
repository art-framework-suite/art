#ifndef art_Framework_Principal_GroupFactory_h
#define art_Framework_Principal_GroupFactory_h
// vim: set sw=2:

#include "art/Framework/Principal/fwd.h"
#include "cetlib/exempt_ptr.h"
#include <memory>

namespace art {

  class BranchDescription;
  class EDProduct;
  class ProductID;
  class RangeSet;

  namespace gfactory {

    std::unique_ptr<Group>
    make_group(BranchDescription const&,
               ProductID const&,
               RangeSet&&);

    std::unique_ptr<Group>
    make_group(BranchDescription const&,
               ProductID const&,
               RangeSet&&,
               cet::exempt_ptr<Worker> productProducer,
               cet::exempt_ptr<EventPrincipal> onDemandPrincipal);

    std::unique_ptr<Group>
    make_group(std::unique_ptr<EDProduct>&&,
               BranchDescription const&,
               ProductID const&,
               RangeSet&&);

  } // namespace gfactory
} // namespace art

#endif /* art_Framework_Principal_GroupFactory_h */

// Local Variables:
// mode: c++
// End:
