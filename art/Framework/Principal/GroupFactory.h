#ifndef art_Framework_Principal_GroupFactory_h
#define art_Framework_Principal_GroupFactory_h
// vim: set sw=2:

#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/AssnsGroupWithData.h"
#include "art/Framework/Principal/Group.h"
#include "canvas/Persistency/Common/detail/getWrapperTIDs.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"

#include <memory>
#include <vector>

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
               cet::exempt_ptr<Worker> productProducer);

    std::unique_ptr<Group>
    make_group(BranchDescription const&,
               ProductID const&,
               RangeSet&&,
               std::unique_ptr<EDProduct>&&);

  } // namespace gfactory

} // namespace art

inline
std::unique_ptr<art::Group>
art::gfactory::
make_group(BranchDescription const& pd,
           ProductID const& pid,
           RangeSet&& rs)
{
  return detail::make_group(pd, pid, std::move(rs));
}

inline
std::unique_ptr<art::Group>
art::gfactory::
make_group(BranchDescription const& pd,
           ProductID const& pid,
           RangeSet&& rs,
           cet::exempt_ptr<Worker> productProducer)
{
  return detail::make_group(pd, pid, std::move(rs), productProducer);
}

inline
std::unique_ptr<art::Group>
art::gfactory::
make_group(BranchDescription const& pd,
           ProductID const& pid,
           RangeSet&& rs,
           std::unique_ptr<EDProduct>&& edp)
{
  return detail::make_group(pd, pid, std::move(rs), std::move(edp));
}

// Where all the real work is done.
template <typename ... ARGS>
std::unique_ptr<art::Group>
art::gfactory::detail::
make_group(BranchDescription const & pd,
           ARGS && ... args)
{
  std::unique_ptr<Group> result;
  auto tids = art::detail::getWrapperTIDs(pd);
  switch (tids.size()) {
  case 1ull: // Standard Group.
    // Can't use std::make_unique<> because Group's constructor is
    // protected.
    result = std::unique_ptr<Group>(new Group(pd, std::forward<ARGS>(args)...,
                                              tids[0]));
    break;
  case 2ull: // Assns<A, B, void>.
    // Can't use std::make_unique<> because AssnGroup's constructor is
    // protected.
    result =
      std::unique_ptr<AssnsGroup>
      (new AssnsGroup(pd, std::forward<ARGS>(args)..., tids[0], tids[1]));
    break;
  case 4ull: // Assns<A, B, D>.
    // Can't use std::make_unique<> because AssnGroupWithData's
    // constructor is protected.
    result =
      std::unique_ptr<AssnsGroupWithData>
      (new AssnsGroupWithData(pd, std::forward<ARGS>(args)...,
                              tids[0], tids[1], tids[2], tids[3]));
    break;
  default:
    // throw internal error exception
    throw Exception(errors::LogicError, "INTERNAL ART ERROR")
      << "While making groups, internal function getWrapperTIDs() returned an unexpected answer of size "
      << tids.size()
      << ".\n";
  }
  return result;
}
#endif /* art_Framework_Principal_GroupFactory_h */

// Local Variables:
// mode: c++
// End:
