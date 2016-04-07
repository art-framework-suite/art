#include "art/Framework/Principal/GroupFactory.h"
// vim: set sw=2:

#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/Group.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Provenance/TypeTools.h"
#include "canvas/Utilities/WrappedClassName.h"
#include "cetlib/demangle.h"
#include "TClass.h"

#include <iostream>

using namespace std;

namespace art {
namespace gfactory {

unique_ptr<Group>
make_group(BranchDescription const& bd,
           ProductID const& pid,
           ProductRangeSetLookup& prsl)
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
    return unique_ptr<Group>(new AssnsGroup(bd, pid, twid, twpid, prsl));
  }
  auto tw = TClass::GetClass(bd.wrappedName().c_str());
  auto twid = TypeID(tw->GetTypeInfo());
  return unique_ptr<Group>(new Group(bd, pid, twid, prsl));
}

unique_ptr<Group>
make_group(BranchDescription const& bd,
           ProductID const& pid,
           ProductRangeSetLookup& prsl,
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
    return unique_ptr<Group>(new AssnsGroup(bd, pid, twid, twpid, prsl, productProducer,
                                            onDemandPrincipal));
  }
  auto tw = TClass::GetClass(bd.wrappedName().c_str());
  auto twid = TypeID(tw->GetTypeInfo());
  return unique_ptr<Group>(new Group(bd, pid, twid, prsl, productProducer,
                                     onDemandPrincipal));
}

unique_ptr<Group>
make_group(unique_ptr<EDProduct>&& edp,
           BranchDescription const& bd,
           ProductID const& pid,
           bool const rangeSetIDIsSet,
           ProductRangeSetLookup& prsl)
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
    return std::unique_ptr<Group>(new AssnsGroup(move(edp), bd, pid, twid, twpid, rangeSetIDIsSet, prsl));
  }
  auto tw = TClass::GetClass(bd.wrappedName().c_str());
  auto twid = TypeID(tw->GetTypeInfo());
  return unique_ptr<Group>(new Group(move(edp), bd, pid, twid, rangeSetIDIsSet, prsl));
}

} // namespace gfactory
} // namespace art
