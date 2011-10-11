#ifndef art_Framework_Art_run_art_h
#define art_Framework_Art_run_art_h

#include "fhiclcpp/intermediate_table.h"

#include "boost/program_options.hpp"

namespace art {
  int run_art(fhicl::intermediate_table raw_config,
              boost::program_options::variables_map const & vm);
}
#endif /* art_Framework_Art_run_art_h */

// Local Variables:
// mode: c++
// End:
