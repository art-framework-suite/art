#include "art/Framework/Principal/AssnsGroup.h"

art::AssnsGroup::AssnsGroup()
  :
  Group(),
  secondary_wrapper_type_(),
  secondaryProduct_()
{}

art::AssnsGroup::
AssnsGroup(BranchDescription const &bd,
           ProductID const &pid,
           art::TypeID const &primary_wrapper_type,
           art::TypeID const &secondary_wrapper_type,
           cet::exempt_ptr<Worker> productProducer,
           cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
  :
  Group(bd, pid, primary_wrapper_type, productProducer, onDemandPrincipal),
  secondary_wrapper_type_(secondary_wrapper_type),
  secondaryProduct_()
{}

art::AssnsGroup::
AssnsGroup(std::auto_ptr<EDProduct> edp,
           BranchDescription const &bd,
           ProductID const &pid,
           art::TypeID const &primary_wrapper_type,
           art::TypeID const &secondary_wrapper_type)
  :
  Group(edp, bd, pid, primary_wrapper_type),
  secondary_wrapper_type_(secondary_wrapper_type),
  secondaryProduct_()
{}

art::AssnsGroup::
~AssnsGroup()
{}

art::EDProduct const *
art::AssnsGroup::
getIt() const {
  return uniqueProduct();
}

art::EDProduct const *
art::AssnsGroup::
anyProduct() const {
  return secondaryProduct_ ?
    secondaryProduct_.get() :
    Group::uniqueProduct();
}

art::EDProduct const *
art::AssnsGroup::
uniqueProduct() const {
  throw Exception(errors::LogicError, "AmbiguousProduct")
    << "AssnsGroup was asked for a held product (uniqueProduct()) "
    << "without specifying which one was wanted.\n";
}

art::EDProduct const *
art::AssnsGroup::
uniqueProduct(art::TypeID const &wanted_wrapper_type) const {
  return
    (wanted_wrapper_type == secondary_wrapper_type_) ?
    secondaryProduct_.get() :
    Group::uniqueProduct();
}

bool
art::AssnsGroup::
resolveProductIfAvailable(bool fillOnDemand,
                          TypeID const &wanted_wrapper_type) const {
  if (uniqueProduct(wanted_wrapper_type)) return true; // Nothing to do.
  if (productUnavailable()) return false; // Nothing we *can* do.
  // Check partner first.
  std::auto_ptr<EDProduct>
    edp(maybeObtainProductFromPartner(wanted_wrapper_type));
  // Now try to read from disk or execute on-demand.
  if (!edp.get()) edp = obtainDesiredProduct(fillOnDemand, wanted_wrapper_type);
  if (wanted_wrapper_type == secondary_wrapper_type_) {
    if (!edp.get()) {
      // On-demand could have been called: attempt to convert.
      edp = maybeObtainProductFromPartner(wanted_wrapper_type);
    }
    secondaryProduct_.reset(edp.release());
  } else {
    // Want the produced type anyway.
    if (edp.get()) setProduct(edp);
  }
  return uniqueProduct(wanted_wrapper_type);
}

std::auto_ptr<art::EDProduct>
art::AssnsGroup::
maybeObtainProductFromPartner(TypeID const &wanted_wrapper_type) const
{
  TypeID const &other_wrapper_type =
    (wanted_wrapper_type == secondary_wrapper_type_) ?
    producedWrapperType() :
    secondary_wrapper_type_;
  return
    uniqueProduct(other_wrapper_type) ?
    uniqueProduct(other_wrapper_type)->makePartner(wanted_wrapper_type.typeInfo()) :
    std::auto_ptr<EDProduct>();
}
