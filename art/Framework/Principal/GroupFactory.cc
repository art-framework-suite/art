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
//**/cout <<  "\n-----> Begin art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid)" << endl;
        auto const taName = bd.producedClassName();
//**/cout <<  "taName = \"" << taName << "\"" << endl;
        TypeWithDict const ta(taName);
        if (!ta) {
          throwLateDictionaryError(taName);
        }
//**/cout <<  "ta.className() = \"" << ta.className() << "\"" << endl;
        auto const twName = art::wrappedClassName(taName);
//**/cout <<  "twName = \"" << twName << "\"" << endl;
        TypeWithDict tw(twName);
        if (!tw) {
          throwLateDictionaryError(twName);
        }
//**/cout <<  "tw.className() = \"" << tw.className() << "\"" << endl;
        twid = tw.id();
        auto const tpName = name_of_assns_partner(taName);
//**/cout <<  "tpName = \"" << tpName << "\"" << endl;
        TypeWithDict tp(tpName);
        bool isNormalGroup = (tp.category() == TypeWithDict::Category::NONE);
        if (!isNormalGroup) {
          if (!tp) {
            throwLateDictionaryError(tpName);
          }
//**/cout <<  "tp.className() = \"" << tp.className() << "\"" << endl;
          auto const twpName = art::wrappedClassName(tpName);
//**/cout <<  "twpName = \"" << twpName << "\"" << endl;
          TypeWithDict twp(twpName);
          if (!twp) {
            throwLateDictionaryError(twpName);
          }
//**/cout <<  "twp.className() = \"" << twp.className() << "\"" << endl;
         twpid = twp.id();
        }
//**/cout <<  "-----> End   art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid)" << endl;
        return isNormalGroup;
      }

    } // unnamed namespace

    unique_ptr<Group>
    make_group(BranchDescription const& bd, ProductID const& pid)
    {
      //**/cout << "\n-----> Begin art::gfactory::make_group(bd,pid)" << endl;
      TypeID twid;
      TypeID twpid;
      if (getWrapperTIDs(bd, twid, twpid)) {
        //**/cout << "making Group(bd,pid,twid)" << endl;
        unique_ptr<Group> group(new Group(bd, pid, twid));
        //**/cout << "-----> End   art::gfactory::make_group(bd,pid)" << endl;
        return group;
      }
      //**/cout << "making AssnsGroup(bd,pid,twid,twpid)" << endl;
      unique_ptr<Group> group(new AssnsGroup(bd, pid, twid, twpid));
      //**/cout << "-----> End   art::gfactory::make_group(bd,pid)" << endl;
      return group;
    }

    unique_ptr<Group>
    make_group(BranchDescription const& bd, ProductID const& pid,
               cet::exempt_ptr<Worker> productProducer,
               cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
    {
      //**/cout << "\n-----> Begin art::gfactory::make_group(bd,pid,pp,odp)" << endl;
      TypeID twid;
      TypeID twpid;
      if (getWrapperTIDs(bd, twid, twpid)) {
        //**/cout << "making Group(bd,pid,twid,pp,odp)" << endl;
        unique_ptr<Group> group(new Group(bd, pid, twid, productProducer,
                                          onDemandPrincipal));
        //**/cout << "-----> End   art::gfactory::make_group(bd,pid,pp,odp)" << endl;
        return group;
      }
      //**/cout << "making AssnsGroup(bd,pid,twid,twpid,pp,odp)" << endl;
      unique_ptr<Group> group(new AssnsGroup(bd, pid, twid, twpid, productProducer,
                                             onDemandPrincipal));
      //**/cout << "-----> End   art::gfactory::make_group(bd,pid,pp,odp)" << endl;
      return group;
    }

    unique_ptr<Group>
    make_group(unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
               ProductID const& pid)
    {
      //**/cout << "\n-----> Begin art::gfactory::make_group(edp,bd,pid)" << endl;
      TypeID twid;
      TypeID twpid;
      if (getWrapperTIDs(bd, twid, twpid)) {
        //**/cout << "making Group(edp,bd,pid,twid)" << endl;
        unique_ptr<Group> group(new Group(move(edp), bd, pid, twid));
        //**/cout << "\n-----> End   art::gfactory::make_group(edp,bd,pid)" << endl;
        return group;
      }
      //**/cout << "making AssnsGroup(edp,bd,pid,twid,twpid)" << endl;
      unique_ptr<Group> group(new AssnsGroup(move(edp), bd, pid, twid, twpid));
      //**/cout << "\n-----> End   art::gfactory::make_group(edp,bd,pid)" << endl;
      return group;
    }

  } // namespace gfactory
} // namespace art

