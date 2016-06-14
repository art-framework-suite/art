#include "art/Framework/IO/ProductMix/MixContainerTypes.h"
#include "art/Persistency/Common/Wrapper.h"

namespace {
  struct dictionary {
    art::EventIDSequence eids;
    art::Wrapper<art::EventIDSequence> eids_w;
  };
}  // namespace
