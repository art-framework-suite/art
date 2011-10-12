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
  virtual ~DeferredProductGetter();

  virtual EDProduct const *getIt() const;
  virtual EDProduct const *anyProduct() const;
  virtual EDProduct const *uniqueProduct() const;
  virtual EDProduct const *uniqueProduct(TypeID const &) const;
  virtual bool resolveProduct(bool fillOnDemand, TypeID const &) const;
  virtual bool resolveProductIfAvailable(bool fillOnDemand,
                                         TypeID const &) const;

private:
  cet::exempt_ptr<EDProductGetter const> resolveGetter_() const;

  cet::exempt_ptr<EventPrincipal const> groupFinder_;
  ProductID const pid_;
  mutable cet::exempt_ptr<EDProductGetter const> realGetter_;
};  // EDProductGetter
#endif /* art_Framework_Principal_DeferredProductGetter_h */

// Local Variables:
// mode: c++
// End:
