#include "art/Framework/IO/ProductMix/MixContainerTypes.h"
#include "art/Persistency/Common/Wrapper.h"

template class art::Wrapper<art::EventIDSequence>;
//template class art::EventIDSequence;
template class std::vector<art::EventID>;

