#include "art/Framework/Principal/GroupFactory.h"
// vim: set sw=2:

#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/Group.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Provenance/TypeTools.h"
#include "art/Utilities/WrappedClassName.h"
#include "cetlib/demangle.h"
#include "TClass.h"

#include <iostream>

using namespace std;

namespace art {
namespace gfactory {

unique_ptr<Group>
make_group(BranchDescription const& bd, ProductID const& pid)
{
  //cout
  //    << "\n-----> Begin gfactory::make_group(bd,pid)"
  //    << endl;
  auto wn = TClass::GetClass(bd.wrappedName().c_str());
  auto ta = type_of_template_arg(wn, 0);
  //cout
  //    << "wn: "
  //    << wn->GetName()
  //    << endl;
  //if (ta == nullptr) {
  //  cout
  //      << "ta: nullptr"
  //      << endl;
  //}
  //else {
  //  cout
  //      << "ta:              "
  //      << ta->GetName()
  //      << endl;
  //}
  TClass* atype = nullptr;
  if (ta != nullptr) {
    if (string(ta->GetName()).find("art::Assns<") == 0ul) {
      atype = ta;
    }
  }
  if (atype != nullptr) {
    auto tw = TClass::GetClass(wrappedClassName(atype->GetName()).c_str());
    auto twid = TypeID(tw->GetTypeInfo());
    string pn = atype->GetName();
    pn += "::partner_t";
    TClass* p = TClass::GetClass(pn.c_str());
    auto pwn = wrappedClassName(p->GetName());
    auto twp = TClass::GetClass(pwn.c_str());
    auto twpid = TypeID(twp->GetTypeInfo());
    //cout
    //    << "tw:    "
    //    << tw->GetName()
    //    << endl
    //    << "twid:  "
    //    << cet::demangle_symbol(twid.name())
    //    << endl
    //    << "twp:   "
    //    << twp->GetName()
    //    << endl
    //    << "twpid: "
    //    << cet::demangle_symbol(twpid.name())
    //    << endl;
    //cout
    //    << "making AssnsGroup(bd,pid,twid,twpid)"
    //    << endl;
    //cout
    //    << "-----> End   gfactory::make_group(bd,pid)"
    //    << endl;
    return unique_ptr<Group>(new AssnsGroup(bd, pid, twid, twpid));
  }
  auto tw = TClass::GetClass(bd.wrappedName().c_str());
  auto twid = TypeID(tw->GetTypeInfo());
  //cout
  //    << "making Group(bd,pid,twid)"
  //    << endl;
  //cout
  //    << "-----> End   gfactory::make_group(bd,pid)"
  //    << endl;
  return unique_ptr<Group>(new Group(bd, pid, twid));
}

unique_ptr<Group>
make_group(BranchDescription const& bd, ProductID const& pid,
           cet::exempt_ptr<Worker> productProducer,
           cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
{
  auto wn = TClass::GetClass(bd.wrappedName().c_str());
  auto ta = type_of_template_arg(wn, 0);
  TClass* atype = nullptr;
  if (ta != nullptr) {
    if (string(ta->GetName()).find("art::Assns<") == 0ul) {
      atype = ta;
    }
  }
  if (atype != nullptr) {
    auto tw = TClass::GetClass(wrappedClassName(atype->GetName()).c_str());
    auto twid = TypeID(tw->GetTypeInfo());
    string pn = atype->GetName();
    pn += "::partner_t";
    TClass* p = TClass::GetClass(pn.c_str());
    auto pwn = wrappedClassName(p->GetName());
    auto twp = TClass::GetClass(pwn.c_str());
    auto twpid = TypeID(twp->GetTypeInfo());
    return unique_ptr<Group>(new AssnsGroup(bd, pid, twid, twpid, productProducer,
                                            onDemandPrincipal));
  }
  auto tw = TClass::GetClass(bd.wrappedName().c_str());
  auto twid = TypeID(tw->GetTypeInfo());
  return unique_ptr<Group>(new Group(bd, pid, twid, productProducer,
                                     onDemandPrincipal));
}

unique_ptr<Group>
make_group(unique_ptr<EDProduct>&& edp, BranchDescription const& bd,
           ProductID const& pid)
{
  auto wn = TClass::GetClass(bd.wrappedName().c_str());
  auto ta = type_of_template_arg(wn, 0);
  TClass* atype = nullptr;
  if (ta != nullptr) {
    if (string(ta->GetName()).find("art::Assns<") == 0ul) {
      atype = ta;
    }
  }
  if (atype != nullptr) {
    auto tw = TClass::GetClass(wrappedClassName(atype->GetName()).c_str());
    auto twid = TypeID(tw->GetTypeInfo());
    string pn = atype->GetName();
    pn += "::partner_t";
    TClass* p = TClass::GetClass(pn.c_str());
    auto pwn = wrappedClassName(p->GetName());
    auto twp = TClass::GetClass(pwn.c_str());
    auto twpid = TypeID(twp->GetTypeInfo());
    return unique_ptr<Group>(new AssnsGroup(move(edp), bd, pid, twid, twpid));
  }
  auto tw = TClass::GetClass(bd.wrappedName().c_str());
  auto twid = TypeID(tw->GetTypeInfo());
  return unique_ptr<Group>(new Group(move(edp), bd, pid, twid));
}

} // namespace gfactory
} // namespace art

