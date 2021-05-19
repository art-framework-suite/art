#ifndef art_Framework_Art_run_art_h
#define art_Framework_Art_run_art_h
// vim: set sw=2 expandtab :

#include "art/Framework/Art/OptionsHandlers.h"
#include "art/Framework/Core/detail/EnabledModules.h"

#include <string>

namespace art {

  int run_art(int argc,
              char** argv,
              bpo::options_description& all_desc,
              art::OptionsHandlers&& handlers);

  int run_art_string_config(std::string const& config_string);

  int run_art_common_(fhicl::ParameterSet const& main_pset,
                      detail::EnabledModules const& enabled_modules);

} // namespace art

#endif /* art_Framework_Art_run_art_h */

// Local Variables:
// mode: c++
// End:
