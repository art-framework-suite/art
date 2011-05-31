#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/Wrapper.h"

#include "cetlib/map_vector.h"

#include <string>
#include <utility>
#include <vector>

namespace {
  struct dictionary {
    cet::map_vector<std::string> mvs;
    cet::map_vector<std::string>::value_type mvs_v;
    cet::map_vector<std::string>::impl_type mvs_i;

    art::Wrapper<cet::map_vector<std::string> > mvs_w;

    art::Ptr<std::string> ps;
    art::Wrapper<art::Ptr<std::string> > ps_w;

    art::PtrVector<std::string> pvs;
    art::Wrapper<art::PtrVector<std::string> > pvs_w;
  };
}
