#include "art/Persistency/Common/Assns.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/Wrapper.h"
#include "cetlib/map_vector.h"
#include "test/TestObjects/AssnTestData.h"

#include <string>
#include <utility>
#include <vector>

template class art::Ptr<double>;
template class art::Wrapper<art::Ptr<double> >;
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

    art::Ptr<cet::map_vector<std::string>::value_type> pp;
    art::Wrapper<art::Ptr<cet::map_vector<std::string>::value_type> > pp_w;

    art::PtrVector<cet::map_vector<std::string>::value_type> pvp;
    art::Wrapper<art::PtrVector<cet::map_vector<std::string>::value_type> > pvp_w;

    std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > mvvp;
    art::Ptr<cet::map_vector<unsigned int>::value_type> mvvp_p;
    art::Wrapper<std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type> > > mvvp_w;

    art::Wrapper<std::vector<size_t> > vst_w;

    std::vector<arttest::AssnTestData> vat;
    art::Assns<size_t, std::string, arttest::AssnTestData> atd;
    art::Assns<size_t, std::string, void> av;
    art::Wrapper<art::Assns<size_t, std::string, arttest::AssnTestData> > atd_w;

    art::Assns<std::string, size_t, arttest::AssnTestData> atd2;
    art::Assns<std::string, size_t, void> av2;
    art::Wrapper<art::Assns<std::string, size_t, arttest::AssnTestData> > atd_w2;

    art::Assns<std::pair<cet::map_vector_key, std::string>, std::string, arttest::AssnTestData> xx1;
    art::Assns<std::pair<cet::map_vector_key, std::string>, std::string, void> xx2;
    art::Wrapper<art::Assns<std::pair<cet::map_vector_key, std::string>, std::string, arttest::AssnTestData> > xx3;
  };
}

// Local Variables:
// mode: c++
// End:
