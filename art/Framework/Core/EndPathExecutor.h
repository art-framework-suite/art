#ifndef art_Framework_Core_EndPathExecutor_h
#define art_Framework_Core_EndPathExecutor_h
////////////////////////////////////////////////////////////////////////
// EndPathExecutor
//
// Class to handle the execution of the end path. Invoked in all the
// right places by the event processor.
//
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/OutputFileSwitchBoundary.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Principal/RunStopwatch.h"
#include "art/Framework/Principal/Worker.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "cetlib/trim.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <array>
#include <memory>
#include <vector>

namespace art {
  class EndPathExecutor;
  class MasterProductRegistry;
}

class art::EndPathExecutor {
public:
  EndPathExecutor(PathManager & pm,
                  ActionTable & actions,
                  ActivityRegistry & areg,
                  MasterProductRegistry& mpr);

  template <typename T>
  void processOneOccurrence(typename T::MyPrincipal & principal);

  void beginJob();
  void endJob();

  void writeEvent(EventPrincipal& ep);
  void writeSubRun(SubRunPrincipal& srp);
  void writeRun(RunPrincipal& rp);

  void setAuxiliaryRangeSetID(SubRunPrincipal& srp);
  void setAuxiliaryRangeSetID(RunPrincipal& rp);

  void closeAllOutputFiles();
  void openAllOutputFiles(FileBlock & fb);

  void closeSomeOutputFiles(std::size_t const);
  void openSomeOutputFiles(FileBlock const& fb);

  void respondToOpenInputFile(FileBlock const & fb);
  void respondToCloseInputFile(FileBlock const & fb);
  void respondToOpenOutputFiles(FileBlock const & fb);
  void respondToCloseOutputFiles(FileBlock const & fb);

  // Allow output files to close that need to
  void recordOutputClosureRequests();

  bool outputsToCloseAtBoundary(Boundary const) const;
  bool outputsToOpen() const;
  bool someOutputsOpen() const;

  // Return whether a module has reached its maximum count.
  bool terminate() const;

  // Temporarily enable or disable a configured end path module.
  bool setEndPathModuleEnabled(std::string const & label, bool enable);

  // Call selectProducts() on all OutputModules.
  virtual void selectProducts(FileBlock const&);

private:
  using OutputWorkers = std::vector<OutputWorker *>;

  void resetAll();

  template <typename T>
  void runEndPaths(typename T::MyPrincipal &);

  template <class F> void doForAllEnabledWorkers_(F f);
  template <class F> void doForAllEnabledOutputWorkers_(F f);

  PathsInfo & endPathInfo_;
  ActionTable * act_table_;
  ActivityRegistry & actReg_;
  OutputWorkers  outputWorkers_;
  std::array<OutputWorkers, Boundary::NBoundaries()> outputWorkersToClose_ {{}}; // filled by aggregation
  OutputWorkers  outputWorkersToOpen_;
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
  std::unique_ptr<RunStopwatch> const stopwatch {endPathInfo_.runStopwatch(T::isEvent_)};
  if (T::isEvent_) {
    endPathInfo_.addEvent();
  }
  try {
    if (!endPathInfo_.pathPtrs().empty()) {
      endPathInfo_.pathPtrs().front()->processOneOccurrence<T>(ep);
    }
  }
  catch (cet::exception & ex) {
    actions::ActionCodes const action {T::isEvent_ ? act_table_->find(ex.root_cause()) : actions::Rethrow};
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

template <class F>
void
art::EndPathExecutor::doForAllEnabledWorkers_(F fcn)
{
  size_t index {0};
  for (auto const& val : endPathInfo_.workers()) {
    if (workersEnabled_[index++]) { fcn(val.second.get()); }
  }
}

template <class F>
void
art::EndPathExecutor::doForAllEnabledOutputWorkers_(F fcn)
{
  size_t index {0};
  for (auto ow : outputWorkers_ ) {
    if (outputWorkersEnabled_[index++]) { fcn(ow); }
  }
}

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_EndPathExecutor_h */
