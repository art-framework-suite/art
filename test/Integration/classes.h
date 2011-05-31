#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/Wrapper.h"

#include "cetlib/map_vector.h"

#include <string>
#include <utility>
#include <vector>

namespace {
  struct dictionary {
    std::pair<cet::map_vector_key, std::string> pmvks;
    std::vector<std::pair<cet::map_vector_key, std::string> > vpmvks;

    cet::map_vector<std::string> mvs;
    art::Wrapper<cet::map_vector<std::string> > mvs_w;

    art::Ptr<std::string> ps;
    art::Wrapper<art::Ptr<std::string> > ps_w;

    art::PtrVector<std::string> pvs;
    art::Wrapper<art::PtrVector<std::string> > pvs_w;
  };
}
