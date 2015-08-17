#include "art/Framework/Principal/GroupFactory.h"

#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/Group.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Provenance/TypeTools.h"
#include "art/Utilities/WrappedClassName.h"

#include "TClass.h"

namespace {
  TClass *  type_of_wrapper(TClass *  wrapped_item_type) {
    return
      TClass::GetClass(art::wrappedClassName(wrapped_item_type->GetName()).c_str());
  }

  TClass *  type_of_wrapper(art::BranchDescription const &bd) {
    return TClass::GetClass(bd.wrappedName().c_str());
  }

  TClass *  type_of_wrapped_item(art::BranchDescription const &bd) {
    return art::type_of_template_arg(type_of_wrapper(bd), 0);
  }

  TClass *  is_assns(TClass *  wrapped_item_type) {
    return art::is_instantiation_of(wrapped_item_type, "art::Assns") ?
      wrapped_item_type :
      nullptr;
  }

  TClass *  is_assns(art::BranchDescription const &bd) {
    return is_assns(type_of_wrapped_item(bd));
  }

  TClass *  type_of_wrapper_of_other_assns(TClass *  const & assns_type) {
    assert(is_assns(assns_type));
    TClass *  partner(TClass::GetClass((std::string(assns_type->GetName()) + "::partner_t").c_str()));
    assert(partner);
    return TClass::GetClass(art::wrappedClassName(partner->GetName()).c_str());
  }
}

std::unique_ptr<art::Group>
art::gfactory::
make_group(BranchDescription const &bd,
           ProductID const &pid)
{
  TClass *  atype(is_assns(bd));
  return
    std::unique_ptr<Group>
    (atype ?
     new AssnsGroup(bd,
                    pid,
                    art::TypeID(type_of_wrapper(atype)->GetTypeInfo()),
                    art::TypeID(type_of_wrapper_of_other_assns(atype)->GetTypeInfo())) :
     new Group(bd,
               pid,
               art::TypeID(type_of_wrapper(bd)->GetTypeInfo())));
}

std::unique_ptr<art::Group>
art::gfactory::
make_group(BranchDescription const &bd,
           ProductID const &pid,
           cet::exempt_ptr<Worker> productProducer,
           cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
{
  TClass *  atype(is_assns(bd));
  return
    std::unique_ptr<Group>
    (atype ?
     new AssnsGroup(bd,
                    pid,
                    art::TypeID(type_of_wrapper(atype)->GetTypeInfo()),
                    art::TypeID(type_of_wrapper_of_other_assns(atype)->GetTypeInfo()),
                    productProducer,
                    onDemandPrincipal) :
     new Group(bd,
               pid,
               art::TypeID(type_of_wrapper(bd)->GetTypeInfo()),
               productProducer,
               onDemandPrincipal));
}

std::unique_ptr<art::Group>
art::gfactory::
make_group(std::unique_ptr<EDProduct> && edp,
           BranchDescription const &bd,
           ProductID const &pid)
{
  TClass *  atype(is_assns(bd));
  return
    std::unique_ptr<Group>
    (atype ?
     new AssnsGroup(std::move(edp),
                    bd,
                    pid,
                    art::TypeID(type_of_wrapper(atype)->GetTypeInfo()),
                    art::TypeID(type_of_wrapper_of_other_assns(atype)->GetTypeInfo())) :
     new Group(std::move(edp),
               bd,
               pid,
               art::TypeID(type_of_wrapper(bd)->GetTypeInfo())));
}
