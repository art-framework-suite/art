#include "art/Framework/Principal/AssnsGroup.h"
// vim: set sw=2:

#include "cetlib/demangle.h"

art::AssnsGroup::
AssnsGroup()
  : Group()
  , secondary_wrapper_type_()
  , secondaryProduct_()
{
}

art::AssnsGroup::
AssnsGroup(BranchDescription const& bd,
           ProductID const& pid,
           TypeID const& primary_wrapper_type,
           TypeID const& secondary_wrapper_type,
           RangeSet&& rs,
           cet::exempt_ptr<Worker> productProducer,
           cet::exempt_ptr<EventPrincipal> onDemandPrincipal)
  : Group{bd, pid, primary_wrapper_type, std::move(rs), productProducer, onDemandPrincipal}
  , secondary_wrapper_type_(secondary_wrapper_type)
  , secondaryProduct_()
{}

art::AssnsGroup::
AssnsGroup(std::unique_ptr<EDProduct>&& edp,
           BranchDescription const& bd,
           ProductID const& pid,
           TypeID const& primary_wrapper_type,
           TypeID const& secondary_wrapper_type,
           RangeSet&& rs)
  : Group{std::move(edp), bd, pid, primary_wrapper_type, std::move(rs)}
  , secondary_wrapper_type_(secondary_wrapper_type)
  , secondaryProduct_()
{
}

art::EDProduct const*
art::AssnsGroup::
getIt() const
{
  return uniqueProduct();
}

art::EDProduct const*
art::AssnsGroup::
anyProduct() const
{
  return secondaryProduct_ ?  secondaryProduct_.get() : Group::uniqueProduct();
}

art::EDProduct const*
art::AssnsGroup::
uniqueProduct() const
{
  throw Exception(errors::LogicError, "AmbiguousProduct")
      << "AssnsGroup was asked for a held product (uniqueProduct()) "
      << "without specifying which one was wanted.\n";
}

art::EDProduct const*
art::AssnsGroup::
uniqueProduct(TypeID const& wanted_wrapper_type) const
{
  EDProduct const* retval = nullptr;
  if (wanted_wrapper_type == secondary_wrapper_type_) {
    retval = secondaryProduct_.get();
  }
  else {
    retval = Group::uniqueProduct();
  }
  return retval;
}

bool
art::AssnsGroup::
resolveProductIfAvailable(bool fillOnDemand, TypeID const& wanted_wrapper_type) const
{
  if (uniqueProduct(wanted_wrapper_type) != nullptr) {
    return true;
  }
  if (productUnavailable()) {
    return false;
  }
  // We know at this point that our wanted object has not
  // been read or created yet.
  std::unique_ptr<EDProduct> edp;
  if (wanted_wrapper_type == secondary_wrapper_type_) {
    if (Group::uniqueProduct() == nullptr) {
      // Our partner needs to be read or
      // demand produced first.
      edp = obtainDesiredProduct(fillOnDemand, producedWrapperType());
      if (edp.get()) {
        setProduct(std::move(edp));
      }
    }
    if (Group::uniqueProduct() != nullptr) {
      // Our partner has already been read, so call its
      // makePartner function to get what we want.
      edp = Group::uniqueProduct()->makePartner(wanted_wrapper_type.typeInfo());
      if (edp.get() != nullptr) {
        secondaryProduct_ = std::move(edp);
      }
    }
  }
  else {
    // We want the produced type.
    edp = obtainDesiredProduct(fillOnDemand, producedWrapperType());
    if (edp.get()) {
      setProduct(std::move(edp));
    }
  }
  bool retval = false;
  if (uniqueProduct(wanted_wrapper_type) != nullptr) {
    retval = true;
  }
  return retval;
}
