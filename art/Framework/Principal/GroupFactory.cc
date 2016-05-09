#include "art/Framework/Principal/GroupFactory.h"
// vim: set sw=2:

#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/Group.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/demangle.h"
#include "TClass.h"

#include <iostream>

using namespace std;

namespace art {
  namespace gfactory {

    namespace {
      bool
      getWrapperTIDs(BranchDescription const& bd, TypeID& twid, TypeID& twpid)
      {
        auto const taName = bd.producedClassName();
        TypeWithDict const ta(taName);
        if (!ta) {
          throwLateDictionaryError(taName);
        }
        auto const twName = art::wrappedClassName(taName);
        TypeWithDict tw(twName);
        if (!tw) {
          throwLateDictionaryError(twName);
        }
        twid = tw.id();
        auto const tpName = name_of_assns_partner(taName);
        TypeWithDict tp(tpName);
        bool isNormalGroup = (tp.category() == TypeWithDict::Category::NONE);
        if (!isNormalGroup) {
          if (!tp) {
            throwLateDictionaryError(tpName);
          }
          auto const twpName = art::wrappedClassName(tpName);
          TypeWithDict twp(twpName);
          if (!twp) {
            throwLateDictionaryError(twpName);
          }
         twpid = twp.id();
        }
        return isNormalGroup;
      }

    } // unnamed namespace

    unique_ptr<Group>
    make_group(BranchDescription const& bd,
               ProductID const& pid,
               RangeSet&& rs)
    {
      TypeID twid;
      TypeID twpid;
      if (getWrapperTIDs(bd, twid, twpid)) {
        unique_ptr<Group> group(new Group(bd, pid, twid, move(rs)));
        return group;
      }
      unique_ptr<Group> group(new AssnsGroup(bd, pid, twid, twpid, move(rs)));
      return group;
    }

    unique_ptr<Group>
    make_group(BranchDescription const& bd,
               ProductID const& pid,
               RangeSet&& rs,
               cet::exempt_ptr<Worker> productProducer,
               cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
    {
      TypeID twid;
      TypeID twpid;
      if (getWrapperTIDs(bd, twid, twpid)) {
        unique_ptr<Group> group(new Group(bd, pid, twid, move(rs), productProducer,
                                          onDemandPrincipal));
        return group;
      }
      unique_ptr<Group> group(new AssnsGroup(bd, pid, twid, twpid, move(rs), productProducer,
                                             onDemandPrincipal));
      return group;
    }

    unique_ptr<Group>
    make_group(unique_ptr<EDProduct>&& edp,
               BranchDescription const& bd,
               ProductID const& pid,
               RangeSet&& rs)
    {
      TypeID twid;
      TypeID twpid;
      if (getWrapperTIDs(bd, twid, twpid)) {
        unique_ptr<Group> group(new Group(move(edp), bd, pid, twid, move(rs)));
        return group;
      }
      unique_ptr<Group> group(new AssnsGroup(move(edp), bd, pid, twid, twpid, move(rs)));
      return group;
    }

  } // namespace gfactory
} // namespace art
