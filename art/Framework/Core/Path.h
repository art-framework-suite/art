#ifndef art_Framework_Core_Path_h
#define art_Framework_Core_Path_h
// vim: set sw=2 expandtab :

//  An object of this type represents one path in a job configuration.
//  It holds the assigned bit position and the list of workers that are
//  an event must pass through when this parh is processed.  The workers
//  are held in WorkerInPath wrappers so that per path execution statistics
//  can be kept for each worker.

#include "art/Framework/Core/WorkerInPath.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Utilities/CurrentProcessingContext.h"
#include "art/Utilities/Transition.h"
#include "canvas/Persistency/Common/HLTGlobalStatus.h"
#include "canvas/Persistency/Common/HLTPathStatus.h"
#include "canvas/Persistency/Common/HLTenums.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"
#include "hep_concurrency/WaitingTask.h"
#include "hep_concurrency/WaitingTaskList.h"

#include <cstddef>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace art {

class ActivityRegistry;
class EventPrincipal;

class Path {

public: // MEMBER FUNCTIONS -- Special Member Functions

  Path(ActionTable&, ActivityRegistry&, int const streamIndex, int const bitpos, bool const isEndPath,
       std::string const& path_name, std::vector<WorkerInPath>&&, HLTGlobalStatus*) noexcept;

  Path(Path const&) = delete;

  Path& operator=(Path const&) = delete;

public: // MEMBER FUNCTIONS

  int
  streamIndex() const;

  int
  bitPosition() const;

  std::string const&
  name() const;

  std::vector<WorkerInPath> const&
  workersInPath() const;

  hlt::HLTState
  state() const;

  std::size_t
  timesRun() const;

  std::size_t
  timesPassed() const;

  std::size_t
  timesFailed() const;

  std::size_t
  timesExcept() const;

  // Note: threading: Clears the counters of workersInPath.
  void
  clearCounters();

  void
  process(Transition, Principal&);

  void
  process_event_for_endpath(EventPrincipal&, int streamIndex);

  void
  process_event(hep::concurrency::WaitingTask* pathsDoneTask, EventPrincipal&, int streamIndex);

private: // MEMBER FUNCTIONS -- Implementation details

  void
  process_event_idx_asynch(size_t idx, size_t const max_idx, EventPrincipal&, int si, bool should_continue,
                           CurrentProcessingContext*);

  void
  process_event_idx(size_t const idx, size_t const max_idx, EventPrincipal&, int si, bool const should_continue, CurrentProcessingContext*);

  void
  process_event_workerFinished(size_t const idx, size_t const max_idx, EventPrincipal&, int si, bool const should_continue,
                               CurrentProcessingContext*);

  void
  process_event_pathFinished(size_t const idx, EventPrincipal&, int si, bool const should_continue, CurrentProcessingContext*);

private: // MEMBER DATA

  ActionTable&
  actionTable_;

  ActivityRegistry&
  actReg_;

  int
  streamIndex_;

  int
  bitpos_;

  bool
  isEndPath_;

  std::string
  name_;

  // Note: threading: We clear their counters.
  std::vector<WorkerInPath>
  workers_;

  // The PathManager trigger paths info actually owns this.
  // Note: For the end path this will be the nullptr.
  HLTGlobalStatus*
  trptr_;

  CurrentProcessingContext
  cpc_;

private: // MEMBER DATA -- Waiting tasks

  // Tasks waiting for path workers to finish.
  hep::concurrency::WaitingTaskList
  waitingTasks_;

private: // MEMBER DATA -- Atomic part, state and counts

  std::atomic<hlt::HLTState>
  state_{hlt::Ready};

  std::atomic<std::size_t>
  timesRun_{};

  std::atomic<std::size_t>
  timesPassed_{};

  std::atomic<std::size_t>
  timesFailed_{};

  std::atomic<std::size_t>
  timesExcept_{};

};

} // namespace art

#endif /* art_Framework_Core_Path_h */

// Local Variables:
// mode: c++
// End:
