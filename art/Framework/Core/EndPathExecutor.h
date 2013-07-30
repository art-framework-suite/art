#ifndef art_Framework_Core_EndPathExecutor_h
#define art_Framework_Core_EndPathExecutor_h
////////////////////////////////////////////////////////////////////////
// EndPathExecutor
//
// Class to handle the execution of the end path. Invoked in all the
// right places by the event processor.
//
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Principal/RunStopwatch.h"
#include "art/Framework/Principal/Worker.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "cetlib/trim.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <functional>
#include <memory>
#include <vector>

namespace art {
  class EndPathExecutor;
}

class art::EndPathExecutor {
public:
  EndPathExecutor(PathManager & pm,
                  ActionTable & actions,
                  std::shared_ptr<ActivityRegistry> areg);

  template <typename T>
  void processOneOccurrence(typename T::MyPrincipal & principal);

  void beginJob();
  void endJob();

  // Write the subRun
  void writeSubRun(SubRunPrincipal const & srp);

  // Write the run
  void writeRun(RunPrincipal const & rp);

  // Call closeFile() on all OutputModules.
  void closeOutputFiles();

  // Call openFiles() on all OutputModules
  void openOutputFiles(FileBlock & fb);

  // Call respondToOpenInputFile() on all Modules
  void respondToOpenInputFile(FileBlock const & fb);

  // Call respondToCloseInputFile() on all Modules
  void respondToCloseInputFile(FileBlock const & fb);

  // Call respondToOpenOutputFiles() on all Modules
  void respondToOpenOutputFiles(FileBlock const & fb);

  // Call respondToCloseOutputFiles() on all Modules
  void respondToCloseOutputFiles(FileBlock const & fb);

  // Call shouldWeCloseFile() on all OutputModules.
  bool shouldWeCloseOutput() const;

  // Return whether a module has reached its maximum count.
  bool terminate() const;

  // Temporarily enable or disable a configured end path module.
  bool setEndPathModuleEnabled(std::string const & label, bool enable);

private:
  typedef std::vector<OutputWorker *> OutputWorkers;

  void resetAll();

  template <typename T>
  void runEndPaths(typename T::MyPrincipal &);

  void doForAllEnabledWorkers_(std::function<void (Worker *)> func);
  void doForAllEnabledOutputWorkers_(std::function<void (OutputWorker *)> func);

  PathsInfo & endPathInfo_;
  ActionTable * act_table_;
  std::shared_ptr<ActivityRegistry> actReg_;
  OutputWorkers  outputWorkers_;
  std::vector<unsigned char> workersEnabled_;
  std::vector<unsigned char> outputWorkersEnabled_;
};

template <typename T>
void
art::EndPathExecutor::
processOneOccurrence(typename T::MyPrincipal & ep)
{
  this->resetAll();
  // A RunStopwatch, but only if we are processing an event.
  std::unique_ptr<RunStopwatch>
    stopwatch(endPathInfo_.runStopwatch(T::isEvent_));
  if (T::isEvent_) {
    endPathInfo_.addEvent();
  }
  try {
    if (!endPathInfo_.pathPtrs().empty()) {
      endPathInfo_.pathPtrs().front()->processOneOccurrence<T>(ep);
    }
  }
  catch (cet::exception & ex) {
    actions::ActionCodes action = (T::isEvent_ ? act_table_->find(ex.root_cause()) : actions::Rethrow);
    switch (action) {
    case actions::IgnoreCompletely: {
      mf::LogWarning(ex.category())
        << "exception being ignored for current event:\n"
        << cet::trim_right_copy(ex.what(), " \n");
      break;
    }
    default: {
      throw art::Exception(errors::EventProcessorFailure)
        << "An exception occurred during current event processing\n"
        << ex;
    }
    }
  }
  catch (...) {
    mf::LogError("PassingThrough")
      << "an exception occurred during current event processing\n";
    throw;
  }
  endPathInfo_.addPass();
}

inline
void
art::EndPathExecutor::
doForAllEnabledWorkers_(std::function<void (Worker *)> func)
{
  size_t index = 0;
  for (auto const & val : endPathInfo_.workers()) {
    if (workersEnabled_[index++]) {
      func(val.second.get());
    }
  }
}

inline
void
art::EndPathExecutor::
doForAllEnabledOutputWorkers_(std::function<void (OutputWorker *)> func)
{
  size_t index = 0;
  for (auto ow : outputWorkers_) {
    if (outputWorkersEnabled_[index++]) {
      func(ow);
    }
  }
}

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_EndPathExecutor_h */
