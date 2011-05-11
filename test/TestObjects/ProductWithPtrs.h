#ifndef test_TestObjects_ProductWithPtrs_h
#define test_TestObjects_ProductWithPtrs_h

#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"

#include <list>
#include <vector>

namespace arttest {
  class ProductWithPtrs;
}

class arttest::ProductWithPtrs {
public:
  ProductWithPtrs();

  // Observers
  art::PtrVector<double> const &ptrVectorDouble() const;
  std::vector<art::Ptr<double> > const &vectorPtrDouble() const;

  // Modifiers
  art::PtrVector<double> &ptrVectorDouble();
  std::vector<art::Ptr<double> > &vectorPtrDouble();

  // Public to allow tests.
  art::PtrVector<double> pvd_;
  std::vector<art::Ptr<double> > vpd_;
};

inline
arttest::ProductWithPtrs::ProductWithPtrs() {}

inline
art::PtrVector<double> const &
arttest::ProductWithPtrs::
ptrVectorDouble() const {
  return pvd_;
}

inline
std::vector<art::Ptr<double> > const &
arttest::ProductWithPtrs::
vectorPtrDouble() const {
  return vpd_;
}

#endif /* test_TestObjects_ProductWithPtrs_h */

// Local Variables:
// mode: c++
// End:
