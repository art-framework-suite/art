#ifndef art_Framework_Core_run_art_h
#define art_Framework_Core_run_art_h

#include "fhiclcpp/intermediate_table.h"

namespace art {
   int run_art(fhicl::intermediate_table raw_config);
}
#endif
