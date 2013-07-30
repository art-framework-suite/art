#ifndef art_Framework_EventProcessor_detail_writeSummary_h
#define art_Framework_EventProcessor_detail_writeSummary_h
namespace art {
  class PathManager;

  namespace detail {
    void writeSummary(PathManager & pm, bool wantSummary);
  }
}
#endif /* art_Framework_EventProcessor_detail_writeSummary_h */

// Local Variables:
// mode: c++
// End:
