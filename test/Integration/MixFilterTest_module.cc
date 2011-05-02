#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Modules/MixFilter.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Utilities/InputTag.h"

namespace arttest {
  class MixFilterTestDetail;
  typedef art::MixFilter<MixFilterTestDetail> MixFilterTest;
  void ff_mix(std::vector<int const *> const &in,
                int &out,
                art::PtrRemapper const &remap);
}

class arttest::MixFilterTestDetail {
public:
  // Constructor is resposible for registering mix operations with
  // MixHelper::declareMixOp() and bookkeeping products with
  // MixHelperproduces().
  MixFilterTestDetail(fhicl::ParameterSet const &p,
                        art::MixHelper &helper);

  // Optional startEvent(): initialize state for each event,
  void startEvent();

  // Return the number of secondaries to read this time. Declare const
  // if you don't plan to change your class' state.
  size_t nSecondaries() const;

  // Optional.finalizeEvent(): (eg) put bookkeping products in event. Do
  // *not* place mix products into the event: this will already have
  // been done for you.
  void finalizeEvent(art::Event &t);

  // Mix functions to be registered with the MixHelper.
  void mix(std::vector<int const *> const &in,
             int &out,
             art::PtrRemapper const &remap);
  void mixDouble(std::vector<double const *> const &in,
                   double &out,
                   art::PtrRemapper const &remap);
  void mix(std::vector<std::string const *> const &in,
             std::string &out,
             art::PtrRemapper const &remap);
private:
  size_t nSecondaries_;
  bool testRemapper_;
};

arttest::MixFilterTestDetail::
MixFilterTestDetail(fhicl::ParameterSet const &p,
                      art::MixHelper &helper)
  :
  nSecondaries_(p.get<size_t>("nunSecondaries", 1)),
  testRemapper_(p.get<bool>("testRemapper", 1))
{
  helper.produces<std::string>(); // "Bookkeeping"

  helper.declareMixOp<int>
    (art::InputTag("intLabel", ""),
     ff_mix);

  helper.declareMixOp<int>
    (art::InputTag("intLabel", ""),
     &MixFilterTestDetail::mix, this);

  helper.declareMixOp
    (art::InputTag("doubleLabel", ""),
     &MixFilterTestDetail::mixDouble, this);

  helper.declareMixOp<std::string>
    (art::InputTag("stringLabel", ""),
     &MixFilterTestDetail::mix, this);
}

void
arttest::MixFilterTestDetail::
startEvent() {
  // FIXME: placeholder.
}

size_t
arttest::MixFilterTestDetail::
nSecondaries() const {
  return nSecondaries_;
}

void
arttest::MixFilterTestDetail::
finalizeEvent(art::Event &e) {
  e.put(std::auto_ptr<std::string>(new std::string("BlahBlahBlah")));
}

DEFINE_ART_MODULE(arttest::MixFilterTest);
