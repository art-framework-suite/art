#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/MergeFilter.h"
#include "art/Framework/Core/MergeHelper.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Utilities/InputTag.h"

namespace arttest {
  class MergeFilterTestDetail;
  typedef art::MergeFilter<MergeFilterTestDetail> MergeFilterTest;
  void ff_merge(std::vector<int const *> const &in,
                int &out,
                art::PtrRemapper const &remap);
}

class arttest::MergeFilterTestDetail {
public:
  MergeFilterTestDetail(fhicl::ParameterSet const &p,
                        art::MergeHelper &helper);

  void merge(std::vector<int const *> const &in,
             int &out,
             art::PtrRemapper const &remap);
  void mergeDouble(std::vector<double const *> const &in,
                   double &out,
                   art::PtrRemapper const &remap);
  void merge(std::vector<std::string const *> const &in,
             std::string &out,
             art::PtrRemapper const &remap);
private:
  size_t nSecondaries_;
};

arttest::MergeFilterTestDetail::
MergeFilterTestDetail(fhicl::ParameterSet const &p,
                      art::MergeHelper &helper)
  :
  nSecondaries_(p.get<size_t>("nunSecondaries", 1))
{
  helper.producesProvider().produces<std::string>(); // "Bookkeeping"

  helper.declareMergeOp<int>
    (art::InputTag("intLabel", ""),
     ff_merge);

  helper.declareMergeOp<int>
    (art::InputTag("intLabel", ""),
     &MergeFilterTestDetail::merge, this);

  helper.declareMergeOp
    (art::InputTag("doubleLabel", ""),
     &MergeFilterTestDetail::mergeDouble, this);

  helper.declareMergeOp<std::string>
    (art::InputTag("stringLabel", ""),
     &MergeFilterTestDetail::merge, this);
}

DEFINE_ART_MODULE(arttest::MergeFilterTest);
