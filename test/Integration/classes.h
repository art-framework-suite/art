#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Common/Ptr.h"
#include "canvas/Persistency/Common/PtrVector.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "cetlib/map_vector.h"
#include "test/TestObjects/AssnTestData.h"

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

template class std::vector<std::pair<art::RefCore, std::size_t>>;
template class std::pair<art::RefCore, std::size_t>;

template class art::Wrapper<art::Assns<std::string, size_t, arttest::AssnTestData>>;
template class art::Assns<std::string, std::size_t, arttest::AssnTestData>;
template class art::Assns<std::string, std::size_t, void>;
template class std::vector<arttest::AssnTestData>;

template class art::Wrapper<art::Assns<std::size_t, std::string, arttest::AssnTestData>>;
template class art::Assns<std::size_t, std::string, arttest::AssnTestData>;
template class art::Assns<std::size_t, std::string, void>;

template class art::Wrapper<art::Assns<std::pair<cet::map_vector_key, std::string>, std::string, arttest::AssnTestData>>;
template class art::Assns<std::pair<cet::map_vector_key, std::string>, std::string, arttest::AssnTestData>;
template class art::Assns<std::pair<cet::map_vector_key, std::string>, std::string, void>;

template class art::Wrapper<art::Ptr<cet::map_vector<std::string>::value_type>>;
template class art::Ptr<cet::map_vector<std::string>::value_type>;

template class art::Wrapper<art::Ptr<int>>;

template class art::Wrapper<art::Ptr<std::string>>;
template class art::Ptr<std::string>;

template class art::Wrapper<art::PtrVector<cet::map_vector<std::string>::value_type>>;
template class art::PtrVector<cet::map_vector<std::string>::value_type>;
template class std::vector<art::Ptr<cet::map_vector<std::string>::value_type>>;
template class std::vector<art::Ptr<std::string>>;

template class art::Wrapper<art::PtrVector<std::string>>;
template class art::PtrVector<std::string>;

template class art::Wrapper<cet::map_vector<std::string>>;
template class cet::map_vector<std::string>;
//template class cet::map_vector<std::string>::value_type;
template class std::pair<cet::map_vector_key, std::string>;
//template class cet::map_vector<std::string>::impl_type;
template class std::vector<std::string>;

template class art::Wrapper<cet::map_vector<unsigned int>>;
template class cet::map_vector<unsigned int>;
//template class cet::map_vector<unsigned int>::value_type;
template class std::pair<cet::map_vector_key, unsigned int>;
//template class cet::map_vector<unsigned int>::impl_type;
template class std::vector<unsigned int>;

template class art::Wrapper<std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type>>>;
template class std::vector<art::Ptr<cet::map_vector<unsigned int>::value_type>>;
template class art::Ptr<cet::map_vector<unsigned int>::value_type>;

template class art::Wrapper<std::vector<std::size_t>>;
template class std::vector<std::size_t>;

// Local Variables:
// mode: c++
// End:
