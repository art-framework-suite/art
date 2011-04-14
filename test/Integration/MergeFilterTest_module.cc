#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/MergeFilter.h"
#include "art/Framework/Core/MergeHelper.h"
#include "art/Utilities/InputTag.h"
#include "cpp0x/functional"

namespace arttest {
  class MergeFilterTestDetail;
  typedef art::MergeFilter<MergeFilterTestDetail> MergeFilterTest;
}

class arttest::MergeFilterTestDetail {
public:
  MergeFilterTestDetail(fhicl::ParameterSet const &p,
                        art::MergeHelper &helper);

  void merge(std::vector<int> const &in, int &out);
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
  helper.declareMergeOp<int, int>(art::InputTag("intLabel", ""),
                                  std::bind(&MergeFilterTestDetail::merge, std::ref(*this)));
}

DEFINE_ART_MODULE(arttest::MergeFilterTest);
