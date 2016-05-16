#include "art/test/Framework/Core/GroupSelector_t.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include <string>

namespace arttest {
  template class ProdTypeA<std::string>;
  template class ProdTypeB<std::string>;
}

namespace art {
  template class Wrapper<arttest::ProdTypeA<std::string> >;
  template class Wrapper<arttest::ProdTypeB<std::string> >;
}
