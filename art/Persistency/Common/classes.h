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
    edm::Wrapper<edm::HLTPathStatus> dummyx16;
    edm::Wrapper<std::vector<edm::HLTPathStatus> > dummyx17;
    edm::Wrapper<edm::HLTGlobalStatus> dummyx18;
    edm::Wrapper<edm::TriggerResults> dummyx19;

    std::pair<edm::ProductID, unsigned int> ppui1;
    edm::Wrapper<edm::ValueMap<int> > wvm1;
    edm::Wrapper<edm::ValueMap<unsigned int> > wvm2;
    edm::Wrapper<edm::ValueMap<bool> > wvm3;
    edm::Wrapper<edm::ValueMap<float> > wvm4;
    edm::Wrapper<edm::ValueMap<double> > wvm5;

    edm::RNGsnapshot                             dummyRNGsnap;
    std::vector<edm::RNGsnapshot>                dummyVectorRNGsnap;
    edm::Wrapper<std::vector<edm::RNGsnapshot> > dummyWrapperVectorRNGsnap;

  };
}  // namespace
