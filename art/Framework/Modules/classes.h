#include "canvas/Persistency/Common/RNGsnapshot.h"
#include "canvas/Persistency/Common/Wrapper.h"

#include <vector>

template class art::Wrapper<std::vector<art::RNGsnapshot>>;
template class std::vector<art::RNGsnapshot>;

