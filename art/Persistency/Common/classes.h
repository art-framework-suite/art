#include "art/Persistency/Common/BoolCache.h"
#include "art/Persistency/Common/ConstPtrCache.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Common/PtrVectorBase.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/RNGsnapshot.h"
#include "art/Persistency/Common/RefCore.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "art/Persistency/Common/Wrapper.h"
#include <vector>

namespace {
  struct dictionary {
    art::Wrapper<art::HLTPathStatus> dummyx16;
    art::Wrapper<std::vector<art::HLTPathStatus> > dummyx17;
    art::Wrapper<art::HLTGlobalStatus> dummyx18;
    art::Wrapper<art::TriggerResults> dummyx19;

    art::PtrVector<int>                dummypvi;
    art::Wrapper<art::PtrVector<int> > dummypviw;

    art::RNGsnapshot                             dummyRNGsnap;
    std::vector<art::RNGsnapshot>                dummyVectorRNGsnap;
    art::Wrapper<std::vector<art::RNGsnapshot> > dummyWrapperVectorRNGsnap;

  };
}  // namespace
