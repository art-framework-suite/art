#ifndef art_Framework_Core_PathsInfo_h
#define art_Framework_Core_PathsInfo_h

#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/WorkerMap.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"

namespace art {
  class PathsInfo;
}

class art::PathsInfo {
public:

  WorkerMap& workers();
  PathPtrs& pathPtrs();
  HLTGlobalStatus& pathResults();

  void addEvent();
  void addPass();

  WorkerMap const& workers() const;
  PathPtrs const& pathPtrs() const;
  size_t passedEvents() const;
  size_t failedEvents() const;
  size_t totalEvents() const;

private:
  WorkerMap workers_ {};
  PathPtrs pathPtrs_ {};
  HLTGlobalStatus pathResults_ {};

  size_t totalEvents_ {};
  size_t passedEvents_ {};
};

inline
art::WorkerMap&
art::PathsInfo::workers()
{
  return workers_;
}

inline
art::PathPtrs&
art::PathsInfo::pathPtrs()
{
  return pathPtrs_;
}

inline
art::HLTGlobalStatus&
art::PathsInfo::pathResults()
{
  return pathResults_;
}

inline
void
art::PathsInfo::addEvent()
{
  ++totalEvents_;
}

inline
void
art::PathsInfo::addPass()
{
  ++passedEvents_;
}

inline
art::WorkerMap const&
art::PathsInfo::workers() const
{
  return workers_;
}

inline
art::PathPtrs const&
art::PathsInfo::pathPtrs() const
{
  return pathPtrs_;
}

inline
size_t
art::PathsInfo::passedEvents() const
{
  return passedEvents_;
}

inline
size_t
art::PathsInfo::failedEvents() const
{
  return totalEvents_ - passedEvents_;
}

inline
size_t
art::PathsInfo::totalEvents() const
{
  return totalEvents_;
}

#endif /* art_Framework_Core_PathsInfo_h */

// Local Variables:
// mode: c++
// End:
