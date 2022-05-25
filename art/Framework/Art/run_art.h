#ifndef art_Framework_Art_run_art_h
#define art_Framework_Art_run_art_h
// vim: set sw=2 expandtab :

#include "art/Framework/Art/OptionsHandlers.h"

namespace art {

  int run_art(int argc,
              char** argv,
              bpo::options_description& all_desc,
              art::OptionsHandlers&& handlers);

} // namespace art

#endif /* art_Framework_Art_run_art_h */

// Local Variables:
// mode: c++
// End:
