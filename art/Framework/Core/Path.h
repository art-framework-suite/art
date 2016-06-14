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
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Principal/RunStopwatch.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Framework/Core/WorkerInPath.h"
#include "art/Persistency/Common/HLTenums.h"
#include "art/Persistency/Common/TriggerResults.h"
#include "cpp0x/memory"
#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include <string>
#include <vector>

// ----------------------------------------------------------------------

namespace art {
  class Path;
  typedef std::vector<std::unique_ptr<Path> > PathPtrs;
}

class art::Path {
public:
  typedef hlt::HLTState State;

  typedef std::vector<WorkerInPath> WorkersInPath;
  typedef WorkersInPath::size_type        size_type;
  typedef cet::exempt_ptr<HLTGlobalStatus> TrigResPtr;

  Path(int bitpos, std::string const& path_name,
       WorkersInPath workers, // Feel free to use move semantics.
       TrigResPtr pathResults,
       fhicl::ParameterSet const& proc_pset,
       ActionTable& actions,
       ActivityRegistry & reg,
       bool isEndPath);

  template <typename T>
  void processOneOccurrence(typename T::MyPrincipal&);

  int bitPosition() const { return bitpos_; }
  std::string const& name() const { return name_; }

  std::pair<double,double> timeCpuReal() const {
    return std::pair<double,double>(stopwatch_->cpuTime(),stopwatch_->realTime());
  }

  std::pair<double,double> timeCpuReal(unsigned int const i) const {
    return workers_.at(i).timeCpuReal();
  }

  void clearCounters();

  int timesRun() const { return timesRun_; }
  int timesPassed() const { return timesPassed_; }
  int timesFailed() const { return timesFailed_; }
  int timesExcept() const { return timesExcept_; }
  //int abortWorker() const { return abortWorker_; }
  State state() const { return state_; }

  size_type size() const { return workers_.size(); }
  int timesVisited(size_type i) const { return workers_.at(i).timesVisited(); }
  int timesPassed (size_type i) const { return workers_.at(i).timesPassed() ; }
  int timesFailed (size_type i) const { return workers_.at(i).timesFailed() ; }
  int timesExcept (size_type i) const { return workers_.at(i).timesExcept() ; }
  Worker const* getWorker(size_type i) const { return workers_.at(i).getWorker(); }

  void findEventModifiers(std::vector<std::string> &foundLabels) const;
  void findEventObservers(std::vector<std::string> &foundLabels) const;

private:

  void findByModifiesEvent(bool modifies, std::vector<std::string> &foundLabels) const;

  RunStopwatch::StopwatchPointer stopwatch_;
  int timesRun_;
  int timesPassed_;
  int timesFailed_;
  int timesExcept_;
  //int abortWorker_;
  State state_;

  int bitpos_;
  std::string name_;
  TrigResPtr trptr_;
  ActivityRegistry & actReg_;
  ActionTable* act_table_;

  WorkersInPath workers_;

  bool isEndPath_;

  // Helper functions
  // nwrwue = numWorkersRunWithoutUnhandledException (really!)
  bool handleWorkerFailure(cet::exception const& e, int nwrwue, bool isEvent);
  void recordUnknownException(int nwrwue, bool isEvent);
  void recordStatus(int nwrwue, bool isEvent);
  void updateCounters(bool succeed, bool isEvent);
};

namespace art {
  namespace {
    template <typename T>
    class PathSignalSentry {
  public:
      PathSignalSentry(ActivityRegistry & a,
                       std::string const& name,
                       int const& nwrwue,
                       hlt::HLTState const& state) :
        a_(a), name_(name), nwrwue_(nwrwue), state_(state) {
        T::prePathSignal(&a_, name_);
      }
      ~PathSignalSentry() {
        HLTPathStatus status(state_, nwrwue_);
        T::postPathSignal(&a_, name_, status);
      }
  private:
      ActivityRegistry & a_;
      std::string const& name_;
      int const& nwrwue_;
      hlt::HLTState const& state_;
    };
  }
}

template <typename T>
void art::Path::processOneOccurrence(typename T::MyPrincipal& ep)
{
  //Create the PathSignalSentry before the RunStopwatch so that
  // we only record the time spent in the path not from the signal
  int nwrwue = -1;
  std::unique_ptr<PathSignalSentry<T> > signaler(new PathSignalSentry<T>(actReg_, name_, nwrwue, state_));

  // A RunStopwatch, but only if we are processing an event.
  std::unique_ptr<RunStopwatch> stopwatch(T::isEvent_ ? new RunStopwatch(stopwatch_) : 0);

  if (T::isEvent_) {
    ++timesRun_;
  }
  state_ = hlt::Ready;

  // nwrue =  numWorkersRunWithoutUnhandledException
  bool should_continue = true;
  CurrentProcessingContext cpc(&name_, bitPosition(), isEndPath_);

  WorkersInPath::size_type idx = 0;
  // It seems likely that 'nwrwue' and 'idx' can never differ ---
  // if so, we should remove one of them!.
  for (WorkersInPath::iterator i = workers_.begin(), end = workers_.end();
       i != end && should_continue;
       ++i, ++idx) {
    ++nwrwue;
    assert (static_cast<int>(idx) == nwrwue);
    try {
      cpc.activate(idx, i->getWorker()->descPtr());
      should_continue = i->runWorker<T>(ep, &cpc);
    }
    catch(cet::exception& e) {
      // handleWorkerFailure may throw a new exception.
      should_continue = handleWorkerFailure(e, nwrwue, T::isEvent_);
    }
    catch(...) {
      recordUnknownException(nwrwue, T::isEvent_);
      throw;
    }
  }
  updateCounters(should_continue, T::isEvent_);
  recordStatus(nwrwue, T::isEvent_);
}

// ======================================================================

#endif /* art_Framework_Core_Path_h */

// Local Variables:
// mode: c++
// End:
