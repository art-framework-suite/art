#include "test/Utilities/TypeNameBranchName_t.h"
#include "cetlib/map_vector.h"
#include <vector>

namespace arttest {
struct empty {};
struct also_empty {};
} // namespace arttest

template class cet::map_vector<std::string>;
template class std::vector<arttest::MyString>;
