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
  //**/cout << "\n-----> Begin art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid)" << endl;
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
    //**/cout << "tw:    " << tw->GetName() << endl;
    twid = TypeID(tw->GetTypeInfo());
    if (!twid) {
      throw Exception(errors::DictionaryNotFound,
                 "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
          << "Could not find typeid for class: "
          << tw->GetName()
          << '\n';
    }
    //**/cout << "twid:  " << cet::demangle_symbol(twid.name()) << endl;
    //**/cout << "-----> End   art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid)" << endl;
    return true;
  }
  //
  //  It was an assns, make an assns group.
  //
  //**/cout << "ta.tClass(): " << ta.tClass()->GetName() << endl;
  auto tw = TClass::GetClass(bd.wrappedName().c_str());
  if (!tw) {
    throw Exception(errors::DictionaryNotFound,
                 "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find dictionary for wrapped Assns class: "
        << bd.wrappedName()
        << '\n';
  }
  //**/cout << "tw:    " << tw->GetName() << endl;
  twid = TypeID(tw->GetTypeInfo());
  if (!twid) {
    throw Exception(errors::DictionaryNotFound,
                 "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find typeid for wrapped Assns class: "
        << tw->GetName()
        << '\n';
  }
  //**/cout << "twid:  " << cet::demangle_symbol(twid.name()) << endl;
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
  //**/cout << "twp:   " << twp->GetName() << endl;
  twpid = TypeID(twp->GetTypeInfo());
  if (!twpid) {
    throw Exception(errors::DictionaryNotFound,
                 "art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid): ")
        << "Could not find typeid for Assns partner class: "
        << twp->GetName()
        << '\n';
  }
  //**/cout << "twpid: " << cet::demangle_symbol(twpid.name()) << endl;
  //**/cout << "-----> End   art::gfactory::<anonymous>::getWrapperTIDs(bd,twid,twpid)" << endl;
  return false;
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

