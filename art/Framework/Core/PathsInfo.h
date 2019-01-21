#ifndef art_Framework_Core_PathsInfo_h
#define art_Framework_Core_PathsInfo_h

#include "art/Framework/Core/Path.h"
#include "art/Framework/Core/WorkerMap.h"
#include "art/Framework/Core/detail/ModuleFactory.h"
#include "art/Framework/Core/detail/ModuleInPathInfo.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "cetlib/exempt_ptr.h"

namespace art {
  class PathsInfo;
}

class art::PathsInfo {
public:
  explicit PathsInfo(std::size_t const numPaths,
                     detail::ModuleFactory& factory,
                     fhicl::ParameterSet const& procPS,
                     MasterProductRegistry& preg,
                     ProductDescriptions& productsToProduce,
                     ActionTable& actions,
                     ActivityRegistry& areg,
                     bool const parentageEnabled,
                     bool const rangesEnabled,
                     bool const dbEnabled);

  HLTGlobalStatus& pathResults();
  using ModInfos = std::vector<detail::ModuleInPathInfo>;

  void makeAndAppendPath(std::string const& pathName,
                         ModInfos const& modInfos,
                         bool trigResultsNeeded = true);
  void addEvent();
  void addPass();

  WorkerMap const& workers() const;
  PathPtrs const& pathPtrs() const;
  size_t passedEvents() const;
  size_t failedEvents() const;
  size_t totalEvents() const;

private:
  void makeWorker_(detail::ModuleInPathInfo const& mipi,
                   std::vector<WorkerInPath>& pathWorkers);

  cet::exempt_ptr<Worker> makeWorker_(detail::ModuleConfigInfo const& mci);

  WorkerMap workers_{};
  PathPtrs pathPtrs_{};
  HLTGlobalStatus pathResults_;

  size_t totalEvents_{};
  size_t passedEvents_{};

  detail::ModuleFactory& fact_;
  fhicl::ParameterSet const& procPS_;
  MasterProductRegistry& preg_;
  ProductDescriptions& productsToProduce_;
  ActionTable& exceptActions_;
  ActivityRegistry& areg_;
  std::vector<std::string> configErrMsgs_;
  bool const parentageEnabled_{true};
  bool const rangesEnabled_{true};
  bool const dbEnabled_{true};
};

inline art::HLTGlobalStatus&
art::PathsInfo::pathResults()
{
  return pathResults_;
}

inline void
art::PathsInfo::addEvent()
{
  ++totalEvents_;
}

inline void
art::PathsInfo::addPass()
{
  ++passedEvents_;
}

inline art::WorkerMap const&
art::PathsInfo::workers() const
{
  return workers_;
}

inline art::PathPtrs const&
art::PathsInfo::pathPtrs() const
{
  return pathPtrs_;
}

inline size_t
art::PathsInfo::passedEvents() const
{
  return passedEvents_;
}

inline size_t
art::PathsInfo::failedEvents() const
{
  return totalEvents_ - passedEvents_;
}

inline size_t
art::PathsInfo::totalEvents() const
{
  return totalEvents_;
}

#endif /* art_Framework_Core_PathsInfo_h */

// Local Variables:
// mode: c++
// End:
