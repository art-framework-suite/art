#include "art/Persistency/Common/RNGsnapshot.h"
#include "art/Persistency/Common/Wrapper.h"

#include <vector>

namespace {
  struct dictionary {
    std::vector<art::RNGsnapshot> snap;
    art::Wrapper<std::vector<art::RNGsnapshot> > snap_w;
  };
}
