#include "art/Framework/Core/PathsInfo.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Path.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/TaskDebugMacros.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "range/v3/view.hpp"

#include <atomic>
#include <cstddef>
#include <map>
#include <string>

using namespace std;

namespace art {

  PathsInfo::~PathsInfo() = default;

  map<string, std::shared_ptr<Worker>>&
  PathsInfo::workers()
  {
    return workers_;
  }

  map<string, std::shared_ptr<Worker>> const&
  PathsInfo::workers() const
  {
    return workers_;
  }

  void
  PathsInfo::add_path(ActionTable const& actions,
                      ActivityRegistry const& registry,
                      PathContext const& pc,
                      std::vector<WorkerInPath>&& wips,
                      GlobalTaskGroup& task_group)
  {
    if (pc.pathName() == PathContext::end_path()) {
      paths_.emplace_back(
        actions, registry, pc, move(wips), nullptr, task_group);
    } else {
      paths_.emplace_back(
        actions, registry, pc, move(wips), &pathResults_, task_group);
      pathResults_ = HLTGlobalStatus(size(paths_));
    }
    TDEBUG_FUNC_SI(5, pc.scheduleID())
      << "Made path " << std::hex << &paths_.back() << std::dec
      << " pathID: " << to_string(pc.pathID()) << " name: " << pc.pathName();
  }

  vector<Path>&
  PathsInfo::paths()
  {
    return paths_;
  }

  vector<Path> const&
  PathsInfo::paths() const
  {
    return paths_;
  }

  vector<string>
  PathsInfo::pathNames() const
  {
    vector<string> result;
    std::transform(begin(paths_),
                   end(paths_),
                   back_inserter(result),
                   [](auto const& path) { return path.name(); });
    return result;
  }

  void
  PathsInfo::reset()
  {
    for (auto const& worker : workers_ | ranges::views::values) {
      worker->reset();
    }
  }

  void
  PathsInfo::reset_for_event()
  {
    reset();
    pathResults_.reset();
  }

  HLTGlobalStatus&
  PathsInfo::pathResults()
  {
    return pathResults_;
  }

  void
  PathsInfo::incrementTotalEventCount()
  {
    ++totalEvents_;
  }

  void
  PathsInfo::incrementPassedEventCount()
  {
    ++passedEvents_;
  }

  size_t
  PathsInfo::passedEvents() const
  {
    return passedEvents_.load();
  }

  size_t
  PathsInfo::failedEvents() const
  {
    // MT: This is fragile!
    return totalEvents_.load() - passedEvents_.load();
  }

  size_t
  PathsInfo::totalEvents() const
  {
    return totalEvents_.load();
  }

} // namespace art
