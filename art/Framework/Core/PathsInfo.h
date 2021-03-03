#ifndef art_Framework_Core_PathsInfo_h
#define art_Framework_Core_PathsInfo_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Path.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"

#include <atomic>
#include <cstddef>
#include <map>
#include <string>

namespace art {
  class PathsInfo {
  public:
    std::map<std::string, Worker*>& workers();
    std::map<std::string, Worker*> const& workers() const;
    void add_path(ActionTable const&,
                  ActivityRegistry const&,
                  PathContext const&,
                  std::vector<WorkerInPath>&&,
                  GlobalTaskGroup&);
    std::vector<Path>& paths();
    std::vector<Path> const& paths() const;
    HLTGlobalStatus& pathResults();
    void reset();
    void reset_for_event();
    void incrementTotalEventCount();
    void incrementPassedEventCount();
    std::size_t passedEvents() const;
    std::size_t failedEvents() const;
    std::size_t totalEvents() const;

  private:
    // Maps module_label to Worker.
    std::map<std::string, Worker*> workers_{};
    std::vector<Path> paths_{};
    HLTGlobalStatus pathResults_{};
    std::atomic<std::size_t> totalEvents_{};
    std::atomic<std::size_t> passedEvents_{};
  };
} // namespace art

#endif /* art_Framework_Core_PathsInfo_h */

// Local Variables:
// mode: c++
// End:
