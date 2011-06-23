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
    cet::map_vector<unsigned int> mvui;
    cet::map_vector<unsigned int>::value_type mvui_v;
    cet::map_vector<unsigned int>::impl_type mvui_i;
    art::Wrapper<cet::map_vector<unsigned int> > mvui_w;

    art::Ptr<std::string> ps;
    art::Wrapper<art::Ptr<std::string> > ps_w;

    art::Ptr<int> pi;
    art::Wrapper<art::Ptr<int> > pi_w;

    art::PtrVector<std::string> pvs;
    art::Wrapper<art::PtrVector<std::string> > pvs_w;

    art::Ptr<std::pair<cet::map_vector_key, std::string> > pp;
    art::Wrapper<art::Ptr<std::pair<cet::map_vector_key, std::string> > > pp_w;

    art::PtrVector<std::pair<cet::map_vector_key, std::string> > pvp;
    art::Wrapper<art::PtrVector<std::pair<cet::map_vector_key, std::string> > > pvp_w;

    std::vector<art::Ptr<unsigned int> > mvvp;
    art::Ptr<unsigned int> mvvp_p;
    art::Wrapper<std::vector<art::Ptr<unsigned int> > > mvvp_w;
  };
}
