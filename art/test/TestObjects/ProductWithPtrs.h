#ifndef art_test_TestObjects_ProductWithPtrs_h
#define art_test_TestObjects_ProductWithPtrs_h

#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"

#include <list>
#include <vector>

namespace arttest {
  class ProductWithPtrs;
}

class arttest::ProductWithPtrs {
public:
  ProductWithPtrs() = default;

  ProductWithPtrs(art::PtrVector<double> const& pvd,
                  std::vector<art::Ptr<double>> const& vpd);

  // Observers
  art::PtrVector<double> const& ptrVectorDouble() const;
  std::vector<art::Ptr<double>> const& vectorPtrDouble() const;

  // Modifiers
  art::PtrVector<double>& ptrVectorDouble();
  std::vector<art::Ptr<double>>& vectorPtrDouble();

  // Public to allow tests.
  art::PtrVector<double> pvd_{};
  std::vector<art::Ptr<double>> vpd_{};
};

inline arttest::ProductWithPtrs::ProductWithPtrs(
  art::PtrVector<double> const& pvd,
  std::vector<art::Ptr<double>> const& vpd)
  : pvd_(pvd), vpd_(vpd)
{}

inline art::PtrVector<double> const&
arttest::ProductWithPtrs::ptrVectorDouble() const
{
  return pvd_;
}

inline std::vector<art::Ptr<double>> const&
arttest::ProductWithPtrs::vectorPtrDouble() const
{
  return vpd_;
}

inline art::PtrVector<double>&
arttest::ProductWithPtrs::ptrVectorDouble()
{
  return pvd_;
}

inline std::vector<art::Ptr<double>>&
arttest::ProductWithPtrs::vectorPtrDouble()
{
  return vpd_;
}

#endif /* art_test_TestObjects_ProductWithPtrs_h */

// Local Variables:
// mode: c++
// End:
