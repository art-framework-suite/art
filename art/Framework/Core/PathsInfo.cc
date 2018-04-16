#include "art/Framework/Core/PathsInfo.h"
// vim: set sw=2 expandtab :

#include "art/Framework/Core/Path.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"

#include <atomic>
#include <cstddef>
#include <map>
#include <mutex>
#include <string>

using namespace std;

namespace art {

  PathsInfo::~PathsInfo()
  {
    for (auto& path : paths_) {
      delete path;
      path = nullptr;
    }
  }

  PathsInfo::PathsInfo() : workers_{}, paths_{}, pathResults_{}
  {
    totalEvents_ = 0;
    passedEvents_ = 0;
  }

  map<string, Worker*>&
  PathsInfo::workers()
  {
    return workers_;
  }

  map<string, Worker*> const&
  PathsInfo::workers() const
  {
    return workers_;
  }

  vector<Path*>&
  PathsInfo::paths()
  {
    return paths_;
  }

  vector<Path*> const&
  PathsInfo::paths() const
  {
    return paths_;
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
    return totalEvents_.load() - passedEvents_.load();
  }

  size_t
  PathsInfo::totalEvents() const
  {
    return totalEvents_.load();
  }

} // namespace art
