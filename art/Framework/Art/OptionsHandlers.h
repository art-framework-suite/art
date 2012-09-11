#ifndef art_Framework_Art_OptionsHandlers_h
#define art_Framework_Art_OptionsHandlers_h
#ifndef __GCCXML__
#include "art/Framework/Art/OptionsHandler.h"

#include <memory>
#include <vector>

namespace art {
  using OptionsHandlers = std::vector<std::unique_ptr<art::OptionsHandler> >;
}
#endif
#endif /* art_Framework_Art_OptionsHandlers_h */

// Local Variables:
// mode: c++
// End:
