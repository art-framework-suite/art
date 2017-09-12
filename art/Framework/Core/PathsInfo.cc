#include "art/Framework/Core/PathsInfo.h"
// vim: set sw=2 expandtab :

using namespace std;

namespace art {

PathsInfo::
~PathsInfo()
{
  for (auto& path : paths_) {
    delete path;
    path = nullptr;
  }
}

PathsInfo::
PathsInfo()
  : workers_{}
  , paths_{}
  , pathResults_{}
  , totalEvents_{}
  , passedEvents_{}
{
}

map<string, Worker*>&
PathsInfo::
workers()
{
  return workers_;
}

map<string, Worker*> const&
PathsInfo::
workers() const
{
  return workers_;
}

vector<Path*>&
PathsInfo::
paths()
{
  return paths_;
}

vector<Path*> const&
PathsInfo::
paths() const
{
  return paths_;
}

HLTGlobalStatus&
PathsInfo::
pathResults()
{
  return pathResults_;
}

void
PathsInfo::
incrementTotalEventCount()
{
  ++totalEvents_;
}

void
PathsInfo::
incrementPassedEventCount()
{
  ++passedEvents_;
}

size_t
PathsInfo::
passedEvents() const
{
  return passedEvents_;
}

size_t
PathsInfo::
failedEvents() const
{
  return totalEvents_ - passedEvents_;
}

size_t
PathsInfo::
totalEvents() const
{
  return totalEvents_;
}

} // namespace art
