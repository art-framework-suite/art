#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Modules/MergeFilter.h"
#include "art/Framework/IO/ProductMerge/MergeHelper.h"
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
  // Constructor is resposible for registering merge operations with
  // MergeHelper::declareMergeOp() and bookkeeping products with
  // MergeHelperproduces().
  MergeFilterTestDetail(fhicl::ParameterSet const &p,
                        art::MergeHelper &helper);

  // Optional startEvent(): initialize state for each event,
  void startEvent();

  // Return the number of secondaries to read this time. Declare const
  // if you don't plan to change your class' state.
  size_t nSecondaries() const;

  // Optional.finalizeEvent(): (eg) put bookkeping products in event. Do
  // *not* place merge products into the event: this will already have
  // been done for you.
  void finalizeEvent(art::Event &t);

  // Merge functions to be registered with the MergeHelper.
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
  helper.produces<std::string>(); // "Bookkeeping"

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

void
arttest::MergeFilterTestDetail::
startEvent() {
  // FIXME: placeholder.
}

size_t
arttest::MergeFilterTestDetail::
nSecondaries() const {
  return nSecondaries_;
}

void
arttest::MergeFilterTestDetail::
finalizeEvent(art::Event &e) {
  e.put(std::auto_ptr<std::string>(new std::string("BlahBlahBlah")));
}

DEFINE_ART_MODULE(arttest::MergeFilterTest);
