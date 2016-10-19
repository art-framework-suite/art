#ifndef art_Framework_EventProcessor_detail_writeSummary_h
#define art_Framework_EventProcessor_detail_writeSummary_h
namespace art {
  class PathManager;
  class PathsInfo;

  namespace detail {
    void writeSummary(PathManager& pm, bool wantSummary);
    void triggerReport(PathsInfo const& endPathsInfo, PathsInfo const& triggerPathsInfo, bool wantSummary);
    void timeReport(PathsInfo const& endPathsInfo, PathsInfo const& triggerPathsInfo, bool wantSummary);
  }
}
#endif /* art_Framework_EventProcessor_detail_writeSummary_h */

// Local Variables:
// mode: c++
// End:
