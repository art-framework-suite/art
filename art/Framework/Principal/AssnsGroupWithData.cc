#include "art/Framework/Principal/AssnsGroupWithData.h"


art::EDProduct const*
art::AssnsGroupWithData::
anyProduct() const
{
  art::EDProduct const * result = AssnsGroup::anyProduct();
  if (result == nullptr) {
    if ((result = baseProduct_.get()) == nullptr) {
      result = partnerBaseProduct_.get();
    }
  }
  return result;
}

art::EDProduct const*
art::AssnsGroupWithData::
uniqueProduct(TypeID const& wanted_wrapper_type) const
{
  EDProduct const* retval = nullptr;
  if (wanted_wrapper_type == partnerBaseWrapperType_) {
    retval = partnerBaseProduct_.get();
  } else if (wanted_wrapper_type == baseWrapperType_) {
    retval = baseProduct_.get();
  } else {
    retval = AssnsGroup::uniqueProduct(wanted_wrapper_type);
  }
  return retval;
}

bool
art::AssnsGroupWithData::
resolveProductIfAvailable(bool fillOnDemand,
                          TypeID const& wanted_wrapper_type) const
{
  // Prerequisite: ask us for something we can do.
  if(!(wanted_wrapper_type == producedWrapperType() ||
       wanted_wrapper_type == partnerWrapperType() ||
       wanted_wrapper_type == baseWrapperType_ ||
       wanted_wrapper_type == partnerBaseWrapperType_)) {
    throwResolveLogicError(wanted_wrapper_type);
  }

  TypeID const & upstream_wrapper_type =
    (wanted_wrapper_type == partnerBaseWrapperType_ ||
     wanted_wrapper_type == partnerWrapperType()) ?
    partnerWrapperType() :
    producedWrapperType();
  bool result =
    AssnsGroup::resolveProductIfAvailable(fillOnDemand,
                                          upstream_wrapper_type);
  if (!(productUnavailable() ||
        (result = uniqueProduct(wanted_wrapper_type))) &&
      AssnsGroup::uniqueProduct(upstream_wrapper_type)) {
    if (wanted_wrapper_type == baseWrapperType_) {
      result = makePartner(wanted_wrapper_type, baseProduct_);
    } else if (wanted_wrapper_type == partnerBaseWrapperType_) {
      result = makePartner(wanted_wrapper_type, partnerBaseProduct_);
    }
  }
  return result;
}

void
art::AssnsGroupWithData::
removeCachedProduct() const
{
  AssnsGroup::removeCachedProduct();
  baseProduct_.reset();
  partnerBaseProduct_.reset();
}
