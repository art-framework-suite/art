#include "art/Persistency/Common/BoolCache.h"
#include "art/Persistency/Common/ConstPtrCache.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Common/EDProductGetter.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include "art/Persistency/Common/HLTPathStatus.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/PtrVectorBase.h"
#include "art/Persistency/Common/RNGsnapshot.h"
#include "art/Persistency/Common/RefCore.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "art/Persistency/Common/Wrapper.h"

#include <utility>
#include <vector>

template class art::Wrapper<art::HLTPathStatus>;
template class art::Wrapper<std::vector<art::HLTPathStatus>>;
template class art::Wrapper<art::HLTGlobalStatus>;
template class art::Wrapper<art::TriggerResults>;
template class std::vector<art::Ptr<int>>;
template class art::PtrVector<int>;
template class art::Wrapper<art::PtrVector<int>>;
//template class std::vector<std::pair<art::RefCore, size_t>>;
//template class std::pair<art::RefCore, size_t>;

// Local Variables:
// mode: c++
// End:
//
