#ifndef art_Framework_Art_run_art_h
#define art_Framework_Art_run_art_h
#ifndef __GCCXML__
#include "art/Framework/Art/OptionsHandlers.h"
#include "cetlib/filepath_maker.h"

namespace art {
  int run_art(int argc,
              char ** argv,
              bpo::options_description & all_desc,
              cet::filepath_maker & lookupPolicy,
              art::OptionsHandlers && handlers);

  int run_art_string_config(const std::string& config_string);

  int run_art_common_(fhicl::ParameterSet main_pset);
}
#endif
#endif /* art_Framework_Art_run_art_h */

// Local Variables:
// mode: c++
// End:
