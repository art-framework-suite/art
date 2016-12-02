#include "art/Framework/Principal/AssnsGroup.h"
// vim: set sw=2:

#include "cetlib/demangle.h"

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
  art::EDProduct const * result = Group::anyProduct();
  if (result == nullptr) {
    result = partnerProduct_.get();
  }
  return result;
}

art::EDProduct const*
art::AssnsGroup::
uniqueProduct() const
{
  throw Exception(errors::LogicError, "AmbiguousProduct")
    << cet::demangle_symbol(typeid(*this).name())
    << " was asked for a held product (uniqueProduct()) "
    << "without specifying which one was wanted.\n";
}

art::EDProduct const*
art::AssnsGroup::
uniqueProduct(TypeID const& wanted_wrapper_type) const
{
  EDProduct const* retval = nullptr;
  if (wanted_wrapper_type == partnerWrapperType_) {
    retval = partnerProduct_.get();
  }
  else {
    retval = Group::uniqueProduct();
  }
  return retval;
}

bool
art::AssnsGroup::
resolveProductIfAvailable(bool fillOnDemand,
                          TypeID const& wanted_wrapper_type) const
{
  bool result =
    Group::resolveProductIfAvailable(fillOnDemand, producedWrapperType());
  if (!(productUnavailable() ||
        (result = uniqueProduct(wanted_wrapper_type))) &&
      wanted_wrapper_type == partnerWrapperType_ &&
      Group::uniqueProduct() != nullptr) {
    // We know at this point that our wanted object has not been read or
    // created yet.
    auto edp = Group::uniqueProduct()->makePartner(wanted_wrapper_type.typeInfo());
    if ((result = edp.get())) {
      partnerProduct_ = std::move(edp);
    }
  }
  return result;
}

void
art::AssnsGroup::
removeCachedProduct() const
{
  Group::removeCachedProduct();
  partnerProduct_.reset();
}
