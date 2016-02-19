#ifndef test_TestObjects_ProductWithPtrs_h
#define test_TestObjects_ProductWithPtrs_h

#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"

#include <list>
#include <vector>

namespace arttest {
  class ProductWithPtrs;
}

class arttest::ProductWithPtrs {
public:
  ProductWithPtrs();

  ProductWithPtrs(
#ifndef ART_NO_MIX_PTRVECTOR
                  art::PtrVector<double> const &pvd,
#endif
                  std::vector<art::Ptr<double> > const &vpd);

  // Observers
#ifndef ART_NO_MIX_PTRVECTOR
  art::PtrVector<double> const &ptrVectorDouble() const;
#endif
  std::vector<art::Ptr<double> > const &vectorPtrDouble() const;

  // Modifiers
#ifndef ART_NO_MIX_PTRVECTOR
  art::PtrVector<double> &ptrVectorDouble();
#endif
  std::vector<art::Ptr<double> > &vectorPtrDouble();

  // Public to allow tests.
#ifndef ART_NO_MIX_PTRVECTOR
  art::PtrVector<double> pvd_;
#endif
  std::vector<art::Ptr<double> > vpd_;
};

inline
arttest::ProductWithPtrs::
ProductWithPtrs()
  :
#ifndef ART_NO_MIX_PTRVECTOR
  pvd_(),
#endif
  vpd_()
{}

inline
arttest::ProductWithPtrs::
ProductWithPtrs(
#ifndef ART_NO_MIX_PTRVECTOR
                art::PtrVector<double> const &pvd,
#endif
                std::vector<art::Ptr<double> > const &vpd)
  :
#ifndef ART_NO_MIX_PTRVECTOR
  pvd_(pvd),
#endif
  vpd_(vpd)
{}

#ifndef ART_NO_MIX_PTRVECTOR
inline
art::PtrVector<double> const &
arttest::ProductWithPtrs::
ptrVectorDouble() const {
  return pvd_;
}
#endif

inline
std::vector<art::Ptr<double> > const &
arttest::ProductWithPtrs::
vectorPtrDouble() const {
  return vpd_;
}

#ifndef ART_NO_MIX_PTRVECTOR
inline
art::PtrVector<double> &
arttest::ProductWithPtrs::
ptrVectorDouble() {
  return pvd_;
}
#endif

inline
std::vector<art::Ptr<double> > &
arttest::ProductWithPtrs::
vectorPtrDouble() {
  return vpd_;
}

#endif /* test_TestObjects_ProductWithPtrs_h */

// Local Variables:
// mode: c++
// End:
