#ifndef art_Framework_Core_EndPathExecutor_h
#define art_Framework_Core_EndPathExecutor_h
// ======================================================================
// EndPathExecutor
//
// Class to handle the execution of the end path. Invoked in all the
// right places by the event processor.
//
// The RangeSetHandlers manage the RangeSets that are to be assigned
// to (a) the (Sub)RunAuxiliaries and (b) the (Sub)Run products
// produced in the current process.  Since all (Sub)Run
// products/auxiliaries produced in the current process are written to
// all output modules during write(Sub)Run, there is only one relevant
// RangeSet for the (Sub)Run at any given time.  RangeSets
// corresponding to multiple (Sub)Run fragments are aggregated on
// input.
// ======================================================================
#include "art/Framework/Core/OutputFileStatus.h"
#include "art/Framework/Core/OutputFileSwitchBoundary.h"
#include "art/Framework/Core/OutputWorker.h"
#include "art/Framework/Core/PathManager.h"
#include "art/Framework/Core/PathsInfo.h"
#include "art/Framework/Principal/Actions.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/OccurrenceTraits.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/MaybeRunStopwatch.h"
#include "art/Framework/Principal/Worker.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "cetlib/trim.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <vector>

namespace art {
  class EndPathExecutor;
  class MasterProductRegistry;
}

class art::EndPathExecutor {
public:
  EndPathExecutor(PathManager& pm,
                  ActionTable& actions,
                  ActivityRegistry& areg,
                  MasterProductRegistry& mpr);

  template <typename T>
  void process(typename T::MyPrincipal& principal);

  void beginJob();
  void endJob();

  void writeEvent(EventPrincipal& ep);
  void writeSubRun(SubRunPrincipal& srp);
  void writeRun(RunPrincipal& rp);

  void seedRunRangeSet(std::unique_ptr<RangeSetHandler>);
  void seedSubRunRangeSet(std::unique_ptr<RangeSetHandler>);

  void setAuxiliaryRangeSetID(SubRunPrincipal& srp);
  void setAuxiliaryRangeSetID(RunPrincipal& rp);

  void closeAllOutputFiles();
  void openAllOutputFiles(FileBlock & fb);

  void closeSomeOutputFiles();
  void openSomeOutputFiles(FileBlock const& fb);
  void setOutputFileStatus(OutputFileStatus);

  void respondToOpenInputFile(FileBlock const& fb);
  void respondToCloseInputFile(FileBlock const& fb);
  void respondToOpenOutputFiles(FileBlock const& fb);
  void respondToCloseOutputFiles(FileBlock const& fb);

  // Allow output files to close that need to
  void recordOutputClosureRequests(Boundary);
  bool outputsToOpen() const;
  bool outputsToClose() const;
  bool someOutputsOpen() const;
  void incrementInputFileNumber();

  // Return whether a module has reached its maximum count.
  bool terminate() const;

  // Temporarily enable or disable a configured end path module.
  bool setEndPathModuleEnabled(std::string const & label, bool enable);

  // Call selectProducts() on all OutputModules.
  void selectProducts(FileBlock const&);

private:
  using OutputWorkers = std::vector<OutputWorker*>;
  using OutputWorkerSet = std::set<OutputWorker*>;

  void resetAll();

  template <typename T>
  void runEndPaths(typename T::MyPrincipal&);

  template <class F> void doForAllEnabledWorkers_(F f);
  template <class F> void doForAllEnabledOutputWorkers_(F f);

  PathsInfo& endPathInfo_;
  ActionTable* act_table_;
  ActivityRegistry& actReg_;
  OutputWorkers  outputWorkers_;
  OutputWorkerSet outputWorkersToOpen_;
  OutputWorkerSet outputWorkersToClose_ {};
  std::vector<unsigned char> workersEnabled_;
  std::vector<unsigned char> outputWorkersEnabled_;
  OutputFileStatus fileStatus_ {OutputFileStatus::Closed};
  std::unique_ptr<RangeSetHandler> runRangeSetHandler_ {nullptr};
  std::unique_ptr<RangeSetHandler> subRunRangeSetHandler_ {nullptr};
};

template <typename T>
void
art::EndPathExecutor::
process(typename T::MyPrincipal & ep)
{
  this->resetAll();
  auto sentry (endPathInfo_.maybeRunStopwatch<T::level>());
  if (T::level == Level::Event) {
    endPathInfo_.addEvent();
  }
  try {
    if (!endPathInfo_.pathPtrs().empty()) {
      endPathInfo_.pathPtrs().front()->process<T>(ep);
    }
  }
  catch (cet::exception & ex) {
    actions::ActionCodes const action {T::level == Level::Event ? act_table_->find(ex.root_cause()) : actions::Rethrow};
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
