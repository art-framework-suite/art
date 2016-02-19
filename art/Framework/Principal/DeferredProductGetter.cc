#include "art/Framework/Principal/DeferredProductGetter.h"

#include "art/Framework/Principal/EventPrincipal.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"

art::DeferredProductGetter::
DeferredProductGetter(cet::exempt_ptr<EventPrincipal const> groupFinder,
                      ProductID pid)
  :
  groupFinder_(groupFinder),
  pid_(pid),
  realGetter_()
{
}

art::EDProduct const *
art::DeferredProductGetter::
getIt() const
{
  return resolveGetter_()->getIt();
}

art::EDProduct const *
art::DeferredProductGetter::
anyProduct() const
{
  return resolveGetter_()->anyProduct();
}

art::EDProduct const *
art::DeferredProductGetter::
uniqueProduct() const
{
  return resolveGetter_()->uniqueProduct();
}

art::EDProduct const *
art::DeferredProductGetter::
uniqueProduct(TypeID const &tid) const
{
  return resolveGetter_()->uniqueProduct(tid);
}

bool
art::DeferredProductGetter::
resolveProduct(bool fillOnDemand, TypeID const &tid) const
{
  return resolveGetter_()->resolveProduct(fillOnDemand, tid);
}

bool
art::DeferredProductGetter::
resolveProductIfAvailable(bool fillOnDemand, TypeID const &tid) const
{
  return resolveGetter_()->resolveProductIfAvailable(fillOnDemand, tid);
}

cet::exempt_ptr<art::EDProductGetter const>
art::DeferredProductGetter::
resolveGetter_() const
{
  auto result = maybeResolveGetter_();
  if (!result) {
    throw Exception(errors::ProductNotFound)
      << "Product corresponding to ProductID "
      << pid_
      << " not found: possible attempt to resolve a Ptr before its product has been committed.\n";
  }
  return std::move(result);
}

cet::exempt_ptr<art::EDProductGetter const>
art::DeferredProductGetter::
maybeResolveGetter_() const
{
  if (realGetter_) {
    return realGetter_;
  } else if (realGetter_ = groupFinder_->getByProductID(pid_).result().get()) {
    return realGetter_;
  } else {
    return cet::exempt_ptr<EDProductGetter const>();
  }
}
