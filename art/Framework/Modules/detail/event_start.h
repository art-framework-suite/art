#ifndef art_Framework_Art_detail_event_start_h
#define art_Framework_Art_detail_event_start_h

#include "canvas/Persistency/Provenance/IDNumber.h"

#include <string>
#include <tuple>

namespace art {
  namespace detail {
    std::tuple<RunNumber_t, SubRunNumber_t, EventNumber_t> event_start(
                                                                       std::string const& str_num);
  }
}

#endif
