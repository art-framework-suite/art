#include "test/Utilities/TypeNameBranchName_t.h"
#include "cetlib/map_vector.h"

namespace arttest {
  struct empty { };
  struct also_empty { };
}

namespace {
  struct dictionary {
    arttest::MyStrings s;
    cet::map_vector<std::string> mvs;
  };
}
