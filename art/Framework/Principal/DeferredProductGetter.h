#ifndef art_Framework_Principal_DeferredProductGetter_h
#define art_Framework_Principal_DeferredProductGetter_h

#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Utilities/fwd.h"
#include "cetlib/exempt_ptr.h"

class art::DeferredProductGetter : public EDProductGetter {
public:
  DeferredProductGetter(cet::exempt_ptr<EventPrincipal const> groupFinder,
                        ProductID pid);

  bool isReady() const override;
  EDProduct const *getIt() const override;
  EDProduct const *anyProduct() const override;
  EDProduct const *uniqueProduct() const override;
  EDProduct const *uniqueProduct(TypeID const &) const override;
  bool resolveProduct(bool fillOnDemand, TypeID const &) const override;
  bool resolveProductIfAvailable(bool fillOnDemand,
                                         TypeID const &) const override;

private:
  cet::exempt_ptr<EDProductGetter const> resolveGetter_() const;
  cet::exempt_ptr<EDProductGetter const> maybeResolveGetter_() const;

  cet::exempt_ptr<EventPrincipal const> groupFinder_;
  ProductID const pid_;
  mutable cet::exempt_ptr<EDProductGetter const> realGetter_;
};  // EDProductGetter

#ifndef __GCCXML__
inline
bool
art::DeferredProductGetter::
isReady()
const
{
  return (maybeResolveGetter_() != nullptr);
}
#endif

#endif /* art_Framework_Principal_DeferredProductGetter_h */

// Local Variables:
// mode: c++
// End:
