#include "art/Persistency/Common/BoolCache.h"
#include "art/Persistency/Common/ConstPtrCache.h"
//#include "art/Persistency/Common/CopyPolicy.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Common/PtrVectorBase.h"
#include "art/Persistency/Common/RNGsnapshot.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "art/Persistency/Common/ValueMap.h"
#include "art/Persistency/Common/Wrapper.h"
#include "art/Persistency/Provenance/ProductID.h"

#include <vector>


namespace {
  struct dictionary {
    art::Wrapper<art::HLTPathStatus> dummyx16;
    art::Wrapper<std::vector<art::HLTPathStatus> > dummyx17;
    art::Wrapper<art::HLTGlobalStatus> dummyx18;
    art::Wrapper<art::TriggerResults> dummyx19;

    std::pair<art::ProductID, unsigned int> ppui1;
    art::Wrapper<art::ValueMap<int> > wvm1;
    art::Wrapper<art::ValueMap<unsigned int> > wvm2;
    art::Wrapper<art::ValueMap<bool> > wvm3;
    art::Wrapper<art::ValueMap<float> > wvm4;
    art::Wrapper<art::ValueMap<double> > wvm5;

    art::RNGsnapshot                             dummyRNGsnap;
    std::vector<art::RNGsnapshot>                dummyVectorRNGsnap;
    art::Wrapper<std::vector<art::RNGsnapshot> > dummyWrapperVectorRNGsnap;

  };
}  // namespace
