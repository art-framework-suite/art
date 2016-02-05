#ifndef art_Persistency_Common_EDProductGetter_h
#define art_Persistency_Common_EDProductGetter_h

#include "canvas/Persistency/Provenance/ProductID.h"
#include "art/Utilities/fwd.h"

namespace art {
  class EDProduct;
  class EDProductGetter;
}

class art::EDProductGetter {
public:

#ifdef __GCCXML__
  EDProductGetter();
  ~EDProductGetter();
private:
  EDProductGetter(EDProductGetter const&);
  // Cannot ref-qualify assignment because of GCC_XML.
  EDProductGetter& operator=(EDProductGetter const&);
public:

#else
  EDProductGetter() = default;
  EDProductGetter(EDProductGetter const&) = delete;
  EDProductGetter& operator=(EDProductGetter const&) = delete;
  virtual ~EDProductGetter() = default;
#endif

  // Can you even try to resolve the product?
  virtual bool isReady() const = 0;
  virtual EDProduct const *getIt() const = 0;
  virtual EDProduct const *anyProduct() const = 0;
  virtual EDProduct const *uniqueProduct() const = 0;
  virtual EDProduct const *uniqueProduct(TypeID const &) const = 0;
  virtual bool resolveProduct(bool fillOnDemand, TypeID const &) const = 0;
  virtual bool resolveProductIfAvailable(bool fillOnDemand,
                                         TypeID const &) const = 0;
};  // EDProductGetter

#endif /* art_Persistency_Common_EDProductGetter_h */

// Local Variables:
// mode: c++
// End:
