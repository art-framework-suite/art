#include "art/Framework/Principal/GroupFactory.h"

#include "art/Framework/Principal/AssnsGroup.h"
#include "art/Framework/Principal/Group.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Provenance/ReflexTools.h"
#include "art/Utilities/WrappedClassName.h"

#include "TClass.h"
#include "Reflex/Type.h"

namespace {
  Reflex::Type type_of_wrapper(Reflex::Type const &wrapped_item_type) {
    return
      Reflex::Type::ByName(art::wrappedClassName
                           (wrapped_item_type.Name
                            (Reflex::FINAL | Reflex::SCOPED)));
  }

  Reflex::Type type_of_wrapper(art::BranchDescription const &bd) {
    return Reflex::Type::ByName(bd.wrappedName());
  }

  Reflex::Type type_of_wrapped_item(art::BranchDescription const &bd) {
    return art::type_of_template_arg(type_of_wrapper(bd), 0);
  }

  Reflex::Type is_assns(Reflex::Type const &wrapped_item_type) {
    return art::is_instantiation_of(wrapped_item_type, "art::Assns") ?
      wrapped_item_type :
      Reflex::Type();
  }

  Reflex::Type is_assns(art::BranchDescription const &bd) {
    return is_assns(type_of_wrapped_item(bd));
  }

  Reflex::Type type_of_wrapper_of_other_assns(Reflex::Type const & assns_type) {
    assert(is_assns(assns_type));
    Reflex::Type partner(Reflex::Type::ByName(assns_type.Name(Reflex::FINAL | Reflex::SCOPED) + "::partner_t"));
    assert(partner);
    return Reflex::Type::ByName(art::wrappedClassName(partner.Name(Reflex::FINAL | Reflex::SCOPED)));
  }
}

std::auto_ptr<art::Group>
art::gfactory::
make_group(BranchDescription const &bd,
           ProductID const &pid)
{
  Reflex::Type atype(is_assns(bd));
  return
    std::auto_ptr<Group>
    (atype ?
     new AssnsGroup(bd,
                    pid,
                    art::TypeID(type_of_wrapper(atype).TypeInfo()),
                    art::TypeID(type_of_wrapper_of_other_assns(atype).TypeInfo())) :
     new Group(bd,
               pid,
               art::TypeID(type_of_wrapper(bd).TypeInfo())));
}

std::auto_ptr<art::Group>
art::gfactory::
make_group(BranchDescription const &bd,
           ProductID const &pid,
           cet::exempt_ptr<Worker> productProducer,
           cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
{
  Reflex::Type atype(is_assns(bd));
  return
    std::auto_ptr<Group>
    (atype ?
     new AssnsGroup(bd,
                    pid,
                    art::TypeID(type_of_wrapper(atype).TypeInfo()),
                    art::TypeID(type_of_wrapper_of_other_assns(atype).TypeInfo()),
                    productProducer,
                    onDemandPrincipal) :
     new Group(bd,
               pid,
               art::TypeID(type_of_wrapper(bd).TypeInfo()),
               productProducer,
               onDemandPrincipal));
}

std::auto_ptr<art::Group>
art::gfactory::
make_group(std::unique_ptr<EDProduct> && edp,
           BranchDescription const &bd,
           ProductID const &pid)
{
  Reflex::Type atype(is_assns(bd));
  return
    std::auto_ptr<Group>
    (atype ?
     new AssnsGroup(edp,
                    bd,
                    pid,
                    art::TypeID(type_of_wrapper(atype).TypeInfo()),
                    art::TypeID(type_of_wrapper_of_other_assns(atype).TypeInfo())) :
     new Group(edp,
               bd,
               pid,
               art::TypeID(type_of_wrapper(bd).TypeInfo())));
}
