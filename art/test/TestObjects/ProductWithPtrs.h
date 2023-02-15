#ifndef art_test_TestObjects_ProductWithPtrs_h
#define art_test_TestObjects_ProductWithPtrs_h

#include "canvas/Persistency/Common/ProductPtr.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"

#include <list>
#include <vector>

namespace arttest {
  class ProductWithPtrs {
  public:
    ProductWithPtrs() = default;

    ProductWithPtrs(art::PtrVector<double> pvd,
                    std::vector<art::Ptr<double>> vpd,
                    art::ProductPtr<std::vector<double>> ppvd);

    // Observers
    art::PtrVector<double> const& ptrVectorDouble() const;
    std::vector<art::Ptr<double>> const& vectorPtrDouble() const;

    // Modifiers
    art::PtrVector<double>& ptrVectorDouble();
    std::vector<art::Ptr<double>>& vectorPtrDouble();

    // Public to allow tests.
    art::PtrVector<double> pvd_{};
    std::vector<art::Ptr<double>> vpd_{};
    art::ProductPtr<std::vector<double>> ppvd_{};
  };

  inline ProductWithPtrs::ProductWithPtrs(
    art::PtrVector<double> pvd,
    std::vector<art::Ptr<double>> vpd,
    art::ProductPtr<std::vector<double>> ppvd)
    : pvd_{std::move(pvd)}, vpd_{std::move(vpd)}, ppvd_{std::move(ppvd)}
  {}

  inline art::PtrVector<double> const&
  ProductWithPtrs::ptrVectorDouble() const
  {
    return pvd_;
  }

  inline std::vector<art::Ptr<double>> const&
  ProductWithPtrs::vectorPtrDouble() const
  {
    return vpd_;
  }

  inline art::PtrVector<double>&
  ProductWithPtrs::ptrVectorDouble()
  {
    return pvd_;
  }

  inline std::vector<art::Ptr<double>>&
  ProductWithPtrs::vectorPtrDouble()
  {
    return vpd_;
  }
}

#endif /* art_test_TestObjects_ProductWithPtrs_h */

// Local Variables:
// mode: c++
// End:
