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
using namespace art;
namespace {

  bool
  getWrapperTIDs(BranchDescription const& bd, TypeID& twid, TypeID& twpid)
  {
    TypeWithDict ta(bd.producedClassName());
    if (!ta) {
      throw Exception(errors::DictionaryNotFound,
                      "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find dictionary for: "
        << bd.producedClassName()
        << '\n';
    }
    if ((ta.category() != TypeWithDict::Category::CLASSTYPE) ||
        (string(ta.tClass()->GetName()).find("art::Assns<") != 0ul)) {
      // Not an assns, make a normal group.
      auto tw = TClass::GetClass(bd.wrappedName().c_str());
      if (!tw) {
        throw Exception(errors::DictionaryNotFound,
                        "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
          << "Could not find dictionary for wrapped class: "
          << bd.wrappedName()
          << '\n';
      }
      twid = TypeID(tw->GetTypeInfo());
      if (!twid) {
        throw Exception(errors::DictionaryNotFound,
                        "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
          << "Could not find typeid for class: "
          << tw->GetName()
          << '\n';
      }
      return true;
    }
    //
    //  It was an assns, make an assns group.
    //
    auto tw = TClass::GetClass(bd.wrappedName().c_str());
    if (!tw) {
      throw Exception(errors::DictionaryNotFound,
                      "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find dictionary for wrapped Assns class: "
        << bd.wrappedName()
        << '\n';
    }
    twid = TypeID(tw->GetTypeInfo());
    if (!twid) {
      throw Exception(errors::DictionaryNotFound,
                      "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find typeid for wrapped Assns class: "
        << tw->GetName()
        << '\n';
    }
    TypeWithDict arg0;
    if (!type_of_template_arg(ta.tClass(), 0, arg0)) {
      throw Exception(errors::DictionaryNotFound,
                      "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find dictionary for template arg L (Assns<L,R,D>) of: "
        << ta.tClass()->GetName()
        << '\n';
    }
    TypeWithDict arg1;
    if (!type_of_template_arg(ta.tClass(), 1, arg1)) {
      throw Exception(errors::DictionaryNotFound,
                      "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find dictionary for template arg R (Assns<L,R,D>) of: "
        << ta.tClass()->GetName()
        << '\n';
    }
    TypeWithDict arg2;
    if (!type_of_template_arg(ta.tClass(), 2, arg2)) {
      throw Exception(errors::DictionaryNotFound,
                      "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find dictionary for template arg D (Assns<L,R,D>) of: "
        << ta.tClass()->GetName()
        << '\n';
    }
    // Make the partner name.
    string pn("art::Assns<");
    pn += arg1.className();
    pn += ',';
    pn += arg0.className();
    pn += ',';
    pn += arg2.className();
    pn += '>';
    //**/cout << "pn: " << pn << endl;
    auto pwn = wrappedClassName(pn);
    auto twp = TClass::GetClass(pwn.c_str());
    if (twp == nullptr) {
      throw Exception(errors::DictionaryNotFound,
                      "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find dictionary for Assns partner class: "
        << pwn
        << '\n';
    }
    twpid = TypeID(twp->GetTypeInfo());
    if (!twpid) {
      throw Exception(errors::DictionaryNotFound,
                      "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find typeid for Assns partner class: "
        << twp->GetName()
        << '\n';
    }
    return false;
  }

} // unnamed namespace

namespace art {
  namespace gfactory {

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
        unique_ptr<Group> group(new Group(bd, pid, twid, move(rs),productProducer,
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
      unique_ptr<Group> group(new AssnsGroup(move(edp), bd, pid, twid, twpid, std::move(rs)));
      return group;
    }

  } // namespace gfactory
} // namespace art
