#ifndef art_Framework_EventProcessor_detail_writeSummary_h
#define art_Framework_EventProcessor_detail_writeSummary_h
// vim: set sw=2 expandtab :

#include "art/Utilities/PerScheduleContainer.h"

namespace cet {
  class cpu_timer;
} // namespace cet

namespace art {

  class PathManager;
  class PathsInfo;

  namespace detail {

    void writeSummary(PathManager& pm,
                      bool wantSummary,
                      cet::cpu_timer const& timer);
    void triggerReport(PerScheduleContainer<PathsInfo> const& endPathInfo,
                       PerScheduleContainer<PathsInfo> const& triggerPathsInfo,
                       bool wantSummary);
    void timeReport(cet::cpu_timer const& timer);

  } // namespace detail

} // namespace art

#endif /* art_Framework_EventProcessor_detail_writeSummary_h */

// Local Variables:
// mode: c++
// End:
