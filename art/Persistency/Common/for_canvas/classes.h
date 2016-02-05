#include "canvas/Persistency/Common/BoolCache.h"
#include "canvas/Persistency/Common/ConstPtrCache.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Common/EDProductGetter.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Persistency/Common/PtrVectorBase.h"
#include "canvas/Persistency/Common/RNGsnapshot.h"
#include "canvas/Persistency/Common/RefCore.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "canvas/Persistency/Common/Wrapper.h"

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
