#include "art/Persistency/Common/RNGsnapshot.h"
#include "art/Persistency/Common/Wrapper.h"

#include <vector>

template class art::Wrapper<std::vector<art::RNGsnapshot>>;
template class std::vector<art::RNGsnapshot>;

