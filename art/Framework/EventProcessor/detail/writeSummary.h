#ifndef art_Framework_EventProcessor_detail_writeSummary_h
#define art_Framework_EventProcessor_detail_writeSummary_h
namespace cet {
  class cpu_timer;
}

namespace art {
  class PathManager;
  class PathsInfo;

  namespace detail {
    void writeSummary(PathManager& pm, bool wantSummary, cet::cpu_timer const& timer);
    void triggerReport(PathsInfo const& endPathsInfo, PathsInfo const& triggerPathsInfo, bool wantSummary);
    void timeReport(cet::cpu_timer const& timer);
  }
}
#endif /* art_Framework_EventProcessor_detail_writeSummary_h */

// Local Variables:
// mode: c++
// End:
