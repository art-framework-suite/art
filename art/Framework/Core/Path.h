#ifndef art_Framework_Core_Path_h
#define art_Framework_Core_Path_h

/*
  An object of this type represents one path in a job configuration.
  It holds the assigned bit position and the list of workers that are
  an event must pass through when this parh is processed.  The workers
  are held in WorkerInPath wrappers so that per path execution statistics
  can be kept for each worker.
*/

#include "art/Framework/Principal/CurrentProcessingContext.h"
#include "art/Framework/Principal/PrincipalPackages.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "canvas/Persistency/Common/HLTenums.h"
#include "canvas/Persistency/Common/TriggerResults.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

// ----------------------------------------------------------------------

namespace art {
  class Path;
  using PathPtrs = std::vector<std::unique_ptr<Path>>;
}

class art::Path {
public:
  using State = hlt::HLTState;

  using WorkersInPath = std::vector<WorkerInPath>;
  using size_type = WorkersInPath::size_type;
  using TrigResPtr = cet::exempt_ptr<HLTGlobalStatus>;

  Path(int bitpos,
       std::string const& path_name,
       WorkersInPath&& workers,
       TrigResPtr pathResults,
       ActionTable const& actions,
       ActivityRegistry& reg,
       bool isEndPath);

  template <typename T>
  void process(typename T::MyPrincipal&);

  int bitPosition() const { return bitpos_; }
  std::string const& name() const { return name_; }

  void clearCounters();

  std::size_t timesRun() const { return timesRun_; }
  std::size_t timesPassed() const { return timesPassed_; }
  std::size_t timesFailed() const { return timesFailed_; }
  std::size_t timesExcept() const { return timesExcept_; }
  State state() const { return state_; }

  auto const& workersInPath() const { return workers_; }

  void findEventModifiers(std::vector<std::string>& foundLabels) const;
  void findEventObservers(std::vector<std::string>& foundLabels) const;

private:

  void findByModifiesEvent(bool modifies, std::vector<std::string>& foundLabels) const;

  std::size_t timesRun_ {};
  std::size_t timesPassed_ {};
  std::size_t timesFailed_ {};
  std::size_t timesExcept_ {};
  State state_ {hlt::Ready};

  int bitpos_;
  std::string name_;
  TrigResPtr trptr_;
  ActivityRegistry& actReg_;
  ActionTable const& act_table_;

  WorkersInPath workers_;

  bool isEndPath_;

  // Helper functions
  // nwrwue = numWorkersRunWithoutUnhandledException (really!)
  bool handleWorkerFailure(cet::exception const& e, int nwrwue, bool isEvent);
  void recordUnknownException(int nwrwue, bool isEvent);
  void recordStatus(int nwrwue, bool isEvent);
  void updateCounters(bool succeed, bool isEvent);
};

template <typename T>
void art::Path::process(typename T::MyPrincipal& ep)
{
  int nwrwue {-1}; // numWorkersRunWithoutUnhandledException
  T::prePathSignal(actReg_, name_);

  if (T::level == Level::Event) {
    ++timesRun_;
  }
  state_ = hlt::Ready;

  bool should_continue {true};
  CurrentProcessingContext cpc {&name_, bitPosition(), isEndPath_};

  for (auto it = workers_.begin(), end = workers_.end(); it != end && should_continue; ++it) {
    ++nwrwue;
    try {
      cpc.activate(nwrwue, it->getWorker()->descPtr());
      should_continue = it->runWorker<T>(ep, &cpc);
    }
    catch (cet::exception& e) {
      // handleWorkerFailure may throw a new exception.
      should_continue = handleWorkerFailure(e, nwrwue, T::level == Level::Event);
    }
    catch (...) {
      recordUnknownException(nwrwue, T::level == Level::Event);
      throw;
    }
  }
  updateCounters(should_continue, T::level == Level::Event);
  recordStatus(nwrwue, T::level == Level::Event);
  HLTPathStatus const status(state_, nwrwue);
  T::postPathSignal(actReg_, name_, status);
}

// ======================================================================

#endif /* art_Framework_Core_Path_h */

// Local Variables:
// mode: c++
// End:
