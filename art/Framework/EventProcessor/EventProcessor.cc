#include "art/Framework/EventProcessor/EventProcessor.h"

#include "art/Framework/Core/Breakpoints.h"
#include "art/Framework/Core/DecrepitRelicInputSourceImplementation.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/InputSourceDescription.h"
#include "art/Framework/Core/InputSourceFactory.h"
#include "art/Framework/EventProcessor/detail/writeSummary.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/PrincipalPackages.h"
#include "art/Framework/Principal/RangeSetHandler.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Framework/Services/System/FloatingPointControl.h"
#include "art/Framework/Services/System/PathSelection.h"
#include "art/Framework/Services/System/ScheduleContext.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/bold_fontify.h"
#include "art/Version/GetReleaseVersion.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProcessConfiguration.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/GetPassID.h"
#include "cetlib/exception_collector.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/types/detail/validationException.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cassert>
#include <exception>
#include <iomanip>
#include <string>
#include <utility>
#include <vector>

using fhicl::ParameterSet;

namespace {

  // Most signals.
  class SignalSentry {
  public:
    using PreSig_t  = art::GlobalSignal<art::detail::SignalResponseType::FIFO, void>;
    using PostSig_t = art::GlobalSignal<art::detail::SignalResponseType::LIFO, void>;

    SignalSentry(SignalSentry const&) = delete;
    SignalSentry& operator=(SignalSentry const&) = delete;

    explicit SignalSentry(PreSig_t& pre, PostSig_t& post) : post_{post}
    {
      pre.invoke();
    }

    ~SignalSentry()
    {
      post_.invoke();
    }

  private:
    PostSig_t& post_;
  };

  ////////////////////////////////////
  void setupAsDefaultEmptySource(ParameterSet& p)
  {
    p.put("module_type", "EmptyEvent");
    p.put("module_label", "source");
    p.put("maxEvents", 1);
  }

  std::unique_ptr<art::InputSource>
  makeInput(ParameterSet const& params,
            std::string const& processName,
            art::MasterProductRegistry& preg,
            art::ActivityRegistry& areg)
  {
    ParameterSet defaultEmptySource;
    setupAsDefaultEmptySource(defaultEmptySource);
    // find single source
    bool sourceSpecified {false};
    ParameterSet main_input {defaultEmptySource};
    try {
      if (!params.get_if_present("source", main_input)) {
        mf::LogInfo("EventProcessorSourceConfig")
          << "Could not find a source configuration: using default.";
      }
      // Fill in "ModuleDescription", in case the input source produces
      // any EDproducts,which would be registered in the
      // MasterProductRegistry.  Also fill in the process history item
      // for this process.
      art::ModuleDescription const md {main_input.id(),
          main_input.get<std::string>("module_type"),
          main_input.get<std::string>("module_label"),
          art::ProcessConfiguration{processName,
            params.id(),
            art::getReleaseVersion(),
            art::getPassID()}};
      sourceSpecified = true;
      art::InputSourceDescription isd{md, preg, areg};
      try {
        return std::unique_ptr<art::InputSource>(art::InputSourceFactory::make(main_input, isd));
      }
      catch(fhicl::detail::validationException const& e){
        throw art::Exception(art::errors::Configuration)
          << "\n\nModule label: " << art::detail::bold_fontify(md.moduleLabel())
          <<   "\nmodule_type : " << art::detail::bold_fontify(md.moduleName())
          << "\n\n" << e.what();
      }
    }
    catch (art::Exception const& x) {
      if (sourceSpecified == false &&
          art::errors::Configuration == x.categoryCode()) {
        throw art::Exception(art::errors::Configuration, "FailedInputSource")
          << "Configuration of main input source has failed\n"
          << x;
      }
      else {
        throw;
      }
    }
    catch (cet::exception const& x) {
      throw art::Exception(art::errors::Configuration, "FailedInputSource")
        << "Configuration of main input source has failed\n"
        << x;
    }
    return std::unique_ptr<art::InputSource>();
  }

  inline std::string spaces(unsigned const n) { return std::string(n,' '); }

}

art::EventProcessor::EventProcessor(ParameterSet const& pset)
  :
  act_table_{pset.get<ParameterSet>("services.scheduler")},
  actReg_(),
  mfStatusUpdater_{actReg_},
  pathManager_{pset, preg_, act_table_, actReg_},
  serviceDirector_{initServices_(pset, actReg_, serviceToken_)},
  handleEmptyRuns_{pset.get<bool>("services.scheduler.handleEmptyRuns", true)},
  handleEmptySubRuns_{pset.get<bool>("services.scheduler.handleEmptySubRuns", true)}
{
  servicesActivate_(serviceToken_);
  serviceToken_.forceCreation();

  std::string const processName {pset.get<std::string>("process_name")};

  // Services
  // System service FileCatalogMetadata needs to know about the process name.
  ServiceHandle<art::FileCatalogMetadata>{}->addMetadataString("process_name", processName);

  input_ = makeInput(pset, processName, preg_, actReg_);
  endPathExecutor_ = std::make_unique<EndPathExecutor>(pathManager_,
                                                       act_table_,
                                                       actReg_,
                                                       preg_);
  // Schedules are created *after* the end-path executor to ensure
  // that ResultsProducers (owned by RootOutput) can call produces<>.
  // The MasterProductRegistry is frozen in the c'tor of the Schedule,
  // which owns all other producers and filters.
  initSchedules_(pset);
  FDEBUG(2) << pset.to_string() << std::endl;
  BranchIDListRegistry::updateFromProductRegistry(preg_);
  servicesDeactivate_();
}

art::EventProcessor::~EventProcessor()
{
  // Services must stay usable until they go out of scope, meaning
  // that modules may (say) use services in their destructors.
  servicesActivate_(serviceToken_);
}

art::ServiceDirector
art::EventProcessor::initServices_(ParameterSet const& top_pset,
                                   ActivityRegistry& areg,
                                   ServiceToken& token)
{
  auto services = top_pset.get<ParameterSet>("services", {});

  // Save and non-standard service configs, "floating_point_control" to
  // prevent ServiceDirector trying to make one itself.
  auto const fpc_pset = services.get<ParameterSet>("floating_point_control", {});
  services.erase("floating_point_control");

  // Remove non-standard non-service config, "message."
  services.erase("message");

  // Deal with possible configuration for system service requiring
  // special construction:
  auto const pathSelection = services.get<ParameterSet>("PathSelection", {});
  services.erase("PathSelection");

  // Create the service director and all user-configured services.
  ServiceDirector director {std::move(services), areg, token};

  // Services requiring special construction.
  director.addSystemService<CurrentModule>(areg);
  director.addSystemService<TriggerNamesService>(top_pset, pathManager_.triggerPathNames());
  director.addSystemService<FloatingPointControl>(fpc_pset, areg);
  director.addSystemService<ScheduleContext>();
  if (!pathSelection.is_empty()) {
    director.addSystemService<PathSelection>(*this);
  }
  return std::move(director);
}

void
art::EventProcessor::initSchedules_(ParameterSet const& pset)
{
  // Initialize TBB with desired number of threads.
  int const num_threads = pset.get<int>("services.num_threads",
                                        tbb::task_scheduler_init::default_num_threads());
  tbbManager_.initialize(num_threads);

  schedule_ = std::make_unique<Schedule>(ScheduleID::first(),
                                         pathManager_,
                                         pset,
                                         *ServiceHandle<TriggerNamesService const>{},
                                         preg_,
                                         act_table_,
                                         actReg_);
}

void
art::EventProcessor::invokePostBeginJobWorkers_()
{
  // Need to convert multiple lists of workers into a long list that the
  // postBeginJobWorkers callbacks can understand.
  std::vector<Worker *> allWorkers;
  allWorkers.reserve(pathManager_.triggerPathsInfo(ScheduleID::first()).workers().size() +
                     pathManager_.endPathInfo().workers().size());
  auto workerStripper = [&allWorkers](WorkerMap::value_type const& val) {
    allWorkers.emplace_back(val.second.get());
  };
  cet::for_all(pathManager_.triggerPathsInfo(ScheduleID::first()).workers(),
               workerStripper);
  cet::for_all(pathManager_.endPathInfo().workers(),
               workerStripper);
  actReg_.sPostBeginJobWorkers.invoke(input_.get(), allWorkers);
}

//================================================================
// Event-loop infrastructure

template <art::Level L>
bool
art::EventProcessor::levelsToProcess()
{
  if (nextLevel_ == Level::ReadyToAdvance) {
    nextLevel_ = advanceItemType();
    boost::mutex::scoped_lock sl {usr2_lock};
    if (art::shutdown_flag > 0) {
      throw Exception{errors::SignalReceived};
    }
    // Consider reading right here?
  }

  if (nextLevel_ == L) {
    activeLevels_.push_back(nextLevel_);
    nextLevel_ = Level::ReadyToAdvance;
    if (endPathExecutor_->outputsToClose()) {
      setOutputFileStatus(OutputFileStatus::Switching);
      finalizeContainingLevels<L>();
      closeSomeOutputFiles();
    }
    return true;
  }
  else if (nextLevel_ < L) {
    return false;
  }
  else if (nextLevel_ == highest_level()) {
    return false;
  }

  throw Exception{errors::LogicError} << "Incorrect level hierarchy.";
}

namespace art {

  // Specializations for process function template

  template <>
  inline void EventProcessor::begin<Level::Job>()
  {
    beginJob();
  }

  template <>
  inline void EventProcessor::begin<Level::InputFile>()
  {
    openInputFile();
  }

  template <>
  void EventProcessor::begin<Level::Run>()
  {
    finalizeRunEnabled_ = true;
    readRun();
    if (handleEmptyRuns_) {
      beginRun();
    }
  }

  template <>
  void EventProcessor::begin<Level::SubRun>()
  {
    finalizeSubRunEnabled_ = true;
    assert(runPrincipal_);
    assert(runPrincipal_->id().isValid());
    readSubRun();
    if (handleEmptySubRuns_) {
      beginRunIfNotDoneAlready();
      beginSubRun();
    }
  }

  template <>
  void EventProcessor::finalize<Level::Event>()
  {
    assert(eventPrincipal_);
    if (eventPrincipal_->id().isFlush()) return;

    openSomeOutputFiles();
    writeEvent();
  }

  template <>
  void EventProcessor::finalize<Level::SubRun>()
  {
    assert(subRunPrincipal_);
    if (!finalizeSubRunEnabled_) return;
    if (subRunPrincipal_->id().isFlush()) return;

    openSomeOutputFiles();
    setSubRunAuxiliaryRangeSetID();
    if (beginSubRunCalled_) {
      endSubRun();
    }
    writeSubRun();

    finalizeSubRunEnabled_ = false;
  }

  template <>
  void EventProcessor::finalize<Level::Run>()
  {
    assert(runPrincipal_);
    if (!finalizeRunEnabled_) return;
    if (runPrincipal_->id().isFlush()) return;

    openSomeOutputFiles();
    setRunAuxiliaryRangeSetID();
    if (beginRunCalled_) {
      endRun();
    }
    writeRun();

    finalizeRunEnabled_ = false;
  }

  template <>
  void EventProcessor::finalize<Level::InputFile>()
  {
    if (nextLevel_ == Level::Job) {
      closeAllFiles();
    }
    else {
      closeInputFile();
    }
  }

  template <>
  inline void EventProcessor::finalize<Level::Job>()
  {
    endJob();
  }

  template <>
  inline void EventProcessor::finalizeContainingLevels<Level::SubRun>()
  {
    finalize<Level::Run>();
  }

  template <>
  inline void EventProcessor::finalizeContainingLevels<Level::Event>()
  {
    finalize<Level::SubRun>();
    finalize<Level::Run>();
  }

  template <>
  inline void EventProcessor::recordOutputModuleClosureRequests<Level::Run>()
  {
    endPathExecutor_->recordOutputClosureRequests(Granularity::Run);
  }

  template <>
  inline void EventProcessor::recordOutputModuleClosureRequests<Level::SubRun>()
  {
    endPathExecutor_->recordOutputClosureRequests(Granularity::SubRun);
  }

  template <>
  inline void EventProcessor::recordOutputModuleClosureRequests<Level::Event>()
  {
    endPathExecutor_->recordOutputClosureRequests(Granularity::Event);
  }

  template <>
  void EventProcessor::process<most_deeply_nested_level()>()
  {
    beginRunIfNotDoneAlready();
    beginSubRunIfNotDoneAlready();
    readEvent();

    assert(eventPrincipal_);
    if (eventPrincipal_->id().isFlush()) return;
    processEvent();

    if (shouldWeStop()) {
      nextLevel_ = highest_level(); // FIXME: maybe go somewhere else?
    }
    finalize<most_deeply_nested_level()>();
    recordOutputModuleClosureRequests<most_deeply_nested_level()>();
  }

} // namespace art

template <art::Level L>
void
art::EventProcessor::process()
{
  begin<L>();
  while (levelsToProcess<level_down(L)>()) {
    process<level_down(L)>();
    markLevelAsProcessed();
  }
  finalize<L>();
  recordOutputModuleClosureRequests<L>();
}

art::EventProcessor::StatusCode
art::EventProcessor::runToCompletion()
{
  StatusCode returnCode {epSuccess};
  // Make the services available
  ServiceRegistry::Operate op {serviceToken_};

  try {
    process<highest_level()>();
  }
  catch (art::Exception const& e) {
    if (e.categoryCode() == art::errors::SignalReceived) {
      returnCode = epSignal;
    }
    else {
      terminateAbnormally_();
      throw e;
    }
  }
  catch (cet::exception const& e) {
    terminateAbnormally_();
    throw e;
  }
  catch (std::bad_alloc const& e) {
    terminateAbnormally_();
    throw cet::exception("std::bad_alloc")
      << "The EventProcessor caught a std::bad_alloc exception and converted it to a cet::exception\n"
      << "The job has probably exhausted the virtual memory available to the process.\n";
  }
  catch (std::exception const& e) {
    terminateAbnormally_();
    throw cet::exception("StdException")
      << "The EventProcessor caught a std::exception and converted it to a cet::exception\n"
      << "Previous information:\n" << e.what() << "\n";
  }
  catch (std::string const& e) {
    terminateAbnormally_();
    throw cet::exception("Unknown")
      << "The EventProcessor caught a string-based exception type and converted it to a cet::exception\n"
      << e
      << "\n";
  }
  catch (char const * e) {
    terminateAbnormally_();
    throw cet::exception("Unknown")
      << "The EventProcessor caught a string-based exception type and converted it to a cet::exception\n"
      << e
      << "\n";
  }
  catch (...) {
    terminateAbnormally_();
    throw cet::exception("Unknown")
      << "The EventProcessor caught an unknown exception type and converted it to a cet::exception\n";
  }
  return returnCode;
}

void
art::EventProcessor::markLevelAsProcessed()
{
  assert(!activeLevels_.empty());
  activeLevels_.pop_back();
}

art::Level
art::EventProcessor::advanceItemType()
{
  auto const itemType = input_->nextItemType();
  FDEBUG(1) << spaces(4) << "*** nextItemType: " << itemType << " ***\n";
  switch(itemType) {
  case input::IsStop: return highest_level();
  case input::IsFile: return Level::InputFile;
  case input::IsRun: return Level::Run;
  case input::IsSubRun: return Level::SubRun;
  case input::IsEvent: return Level::Event;
  case input::IsInvalid: {
    throw art::Exception{art::errors::LogicError}
      << "Invalid next item type presented to the event processor.\n"
      << "Please contact artists@fnal.gov.";
  }
  }
  throw art::Exception{art::errors::LogicError}
    << "Unrecognized next item type presented to the event processor.\n"
    << "Please contact artists@fnal.gov.";
}

//=============================================
// Job level

void
art::EventProcessor::beginJob()
{
  FDEBUG(1) << spaces(8) << "beginJob\n";
  breakpoints::beginJob();
  // make the services available
  ServiceRegistry::Operate op {serviceToken_};
  // NOTE: This implementation assumes 'Job' means one call the
  // EventProcessor::run. If it really means once per 'application'
  // then this code will have to be changed.  Also have to deal with
  // case where have 'run' then new Module added and do 'run' again.
  // In that case the newly added Module needs its 'beginJob' to be
  // called.
  try {
    input_->doBeginJob();
  }
  catch (cet::exception& e) {
    mf::LogError("BeginJob") << "A cet::exception happened while processing"
      " the beginJob of the 'source'\n";
    e << "A cet::exception happened while processing"
      " the beginJob of the 'source'\n";
    throw;
  }
  catch (std::exception const&) {
    mf::LogError("BeginJob") << "A std::exception happened while processing"
      " the beginJob of the 'source'\n";
    throw;
  }
  catch (...) {
    mf::LogError("BeginJob") << "An unknown exception happened while"
      " processing the beginJob of the 'source'\n";
    throw;
  }
  schedule_->beginJob();
  endPathExecutor_->beginJob();
  actReg_.sPostBeginJob.invoke();

  invokePostBeginJobWorkers_();
}

void
art::EventProcessor::endJob()
{
  FDEBUG(1) << spaces(8) << "endJob\n";
  // Collects exceptions, so we don't throw before all operations are performed.
  cet::exception_collector c;
  // Make the services available
  ServiceRegistry::Operate op {serviceToken_};
  c.call([this](){ schedule_->endJob(); });
  c.call([this](){ endPathExecutor_->endJob(); });
  c.call([this](){ detail::writeSummary(pathManager_,
                                        ServiceHandle<TriggerNamesService const>{}->wantSummary()); });
  c.call([this](){ input_->doEndJob(); });
  c.call([this](){ actReg_.sPostEndJob.invoke(); });
}

//====================================================
// File level

void
art::EventProcessor::openInputFile()
{
  actReg_.sPreOpenFile.invoke();
  FDEBUG(1) << spaces(8) << "openInputFile\n";
  fb_ = input_->readFile(preg_);
  if (!fb_) {
    throw Exception(errors::LogicError)
      << "Source readFile() did not return a valid FileBlock: FileBlock "
      << "should be valid or readFile() should throw.\n";
  }
  actReg_.sPostOpenFile.invoke(fb_->fileName());
  respondToOpenInputFile();
}

void
art::EventProcessor::closeAllFiles()
{
  closeAllOutputFiles();
  closeInputFile();
}

void
art::EventProcessor::closeInputFile()
{
  endPathExecutor_->incrementInputFileNumber();
  // Output-file closing on input-file boundaries are tricky since
  // input files must outlive the output files, which often have data
  // copied forward from the input files.  That's why the
  // recordOutputClosureRequests call is made here instead of in a
  // specialization of recordOutputModuleClosureRequests<>.
  endPathExecutor_->recordOutputClosureRequests(Granularity::InputFile);
  if (endPathExecutor_->outputsToClose()) {
    closeSomeOutputFiles();
  }
  respondToCloseInputFile();
  SignalSentry fileCloseSentry {actReg_.sPreCloseFile, actReg_.sPostCloseFile};
  input_->closeFile();
  FDEBUG(1) << spaces(8) << "closeInputFile\n";
}

void
art::EventProcessor::openAllOutputFiles()
{
  endPathExecutor_->openAllOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "openAllOutputFiles\n";
}

void
art::EventProcessor::closeAllOutputFiles()
{
  if (!endPathExecutor_->someOutputsOpen()) return;

  respondToCloseOutputFiles();
  endPathExecutor_->closeAllOutputFiles();
  FDEBUG(1) << spaces(8) << "closeAllOutputFiles\n";
}

void
art::EventProcessor::openSomeOutputFiles()
{
  if (!endPathExecutor_->outputsToOpen()) return;

  endPathExecutor_->openSomeOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "openSomeOutputFiles\n";
  respondToOpenOutputFiles();
}

void
art::EventProcessor::setOutputFileStatus(OutputFileStatus const ofs)
{
  endPathExecutor_->setOutputFileStatus(ofs);
  FDEBUG(1) << spaces(8) << "setOutputFileStatus\n";
}

void
art::EventProcessor::closeSomeOutputFiles()
{
  // Precondition: there are SOME output files that have been
  //               flagged as needing to close.  Otherwise,
  //               'respondtoCloseOutputFiles' will be needlessly
  //               called.
  assert(endPathExecutor_->outputsToClose());
  respondToCloseOutputFiles();
  endPathExecutor_->closeSomeOutputFiles();
  FDEBUG(1) << spaces(8) << "closeSomeOutputFiles\n";
}

void
art::EventProcessor::respondToOpenInputFile()
{
  schedule_->respondToOpenInputFile(*fb_);
  endPathExecutor_->respondToOpenInputFile(*fb_);
  FDEBUG(1) << spaces(8) << "respondToOpenInputFile\n";
}

void
art::EventProcessor::respondToCloseInputFile()
{
  schedule_->respondToCloseInputFile(*fb_);
  endPathExecutor_->respondToCloseInputFile(*fb_);
  FDEBUG(1) << spaces(8) << "respondToCloseInputFile\n";
}

void
art::EventProcessor::respondToOpenOutputFiles()
{
  schedule_->respondToOpenOutputFiles(*fb_);
  endPathExecutor_->respondToOpenOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "respondToOpenOutputFiles\n";
}

void
art::EventProcessor::respondToCloseOutputFiles()
{
  schedule_->respondToCloseOutputFiles(*fb_);
  endPathExecutor_->respondToCloseOutputFiles(*fb_);
  FDEBUG(1) << spaces(8) << "respondToCloseOutputFiles\n";
}

//=============================================
// Run level

void
art::EventProcessor::readRun()
{
  SignalSentry runSourceSentry {actReg_.sPreSourceRun, actReg_.sPostSourceRun};
  runPrincipal_ = input_->readRun();
  endPathExecutor_->seedRunRangeSet(input_->runRangeSetHandler());
  assert(runPrincipal_);
  FDEBUG(1) << spaces(8) << "readRun.....................(" << runPrincipal_->id() << ")\n";
}

void
art::EventProcessor::beginRun()
{
  assert(runPrincipal_);
  RunID const r {runPrincipal_->id()};
  if (r.isFlush()) return;

  finalizeRunEnabled_ = true;
  process_<Begin<Level::Run>>(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "beginRun....................(" << r << ")\n";
  beginRunCalled_ = true;
}

void
art::EventProcessor::beginRunIfNotDoneAlready()
{
  if (!beginRunCalled_) {
    beginRun();
  }
}

void
art::EventProcessor::setRunAuxiliaryRangeSetID()
{
  assert(runPrincipal_);
  endPathExecutor_->setAuxiliaryRangeSetID(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "setRunAuxiliaryRangeSetID...(" << runPrincipal_->id() << ")\n";
}

void
art::EventProcessor::endRun()
{
  assert(runPrincipal_);
  // Precondition: The RunID does not correspond to a flush ID. --
  // N.B. The flush flag is not explicitly checked here since endRun
  // is only called from finalizeRun, which is where the check
  // happens.
  RunID const run {runPrincipal_->id()};
  assert(!run.isFlush());
  process_<End<Level::Run>>(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "endRun......................(" << run << ")\n";
  beginRunCalled_ = false;
}

void
art::EventProcessor::writeRun()
{
  assert(runPrincipal_);
  // Precondition: The RunID does not correspond to a flush ID.
  RunID const r {runPrincipal_->id()};
  assert(!r.isFlush());
  endPathExecutor_->writeRun(*runPrincipal_);
  FDEBUG(1) << spaces(8) << "writeRun....................(" << r << ")\n";
}

//=============================================
// SubRun level

void
art::EventProcessor::readSubRun()
{
  SignalSentry subRunSourceSentry {actReg_.sPreSourceSubRun, actReg_.sPostSourceSubRun};
  subRunPrincipal_ = input_->readSubRun(runPrincipal_.get());
  endPathExecutor_->seedSubRunRangeSet(input_->subRunRangeSetHandler());
  assert(subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "readSubRun..................(" << subRunPrincipal_->id() << ")\n";
}

void
art::EventProcessor::beginSubRun()
{
  assert(subRunPrincipal_);
  SubRunID const sr {subRunPrincipal_->id()};
  if (sr.isFlush()) return;

  finalizeSubRunEnabled_ = true;
  process_<Begin<Level::SubRun>>(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "beginSubRun.................(" << sr <<")\n";
  beginSubRunCalled_ = true;
}

void
art::EventProcessor::beginSubRunIfNotDoneAlready()
{
  if (!beginSubRunCalled_) {
    beginSubRun();
  }
}

void
art::EventProcessor::setSubRunAuxiliaryRangeSetID()
{
  assert(subRunPrincipal_);
  endPathExecutor_->setAuxiliaryRangeSetID(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "setSubRunAuxiliaryRangeSetID(" << subRunPrincipal_->id() << ")\n";
}

void
art::EventProcessor::endSubRun()
{
  assert(subRunPrincipal_);
  // Precondition: The SubRunID does not correspond to a flush ID.
  // Note: the flush flag is not explicitly checked here since
  // endSubRun is only called from finalizeSubRun, which is where the
  // check happens.
  SubRunID const sr {subRunPrincipal_->id()};
  assert(!sr.isFlush());
  process_<End<Level::SubRun>>(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "endSubRun...................(" << sr << ")\n";
  beginSubRunCalled_ = false;
}

void
art::EventProcessor::writeSubRun()
{
  assert(subRunPrincipal_);
  // Precondition: The SubRunID does not correspond to a flush ID.
  SubRunID const& sr {subRunPrincipal_->id()};
  assert(!sr.isFlush());
  endPathExecutor_->writeSubRun(*subRunPrincipal_);
  FDEBUG(1) << spaces(8) << "writeSubRun.................(" << sr << ")\n";
}

//=============================================
// Event level

void
art::EventProcessor::readEvent()
{
  assert(subRunPrincipal_);
  assert(subRunPrincipal_->id().isValid());
  SignalSentry sourceSentry {actReg_.sPreSource, actReg_.sPostSource};
  eventPrincipal_ = input_->readEvent(subRunPrincipal_.get());
  assert(eventPrincipal_);
  FDEBUG(1) << spaces(8) << "readEvent...................(" << eventPrincipal_->id() << ")\n";
}

void
art::EventProcessor::processEvent()
{
  assert(eventPrincipal_);
  EventID const& id {eventPrincipal_->id()};
  // Precondition: The EventID does not correspond to a flush ID.
  assert(!id.isFlush());
  process_<Do<Level::Event>>(*eventPrincipal_);
  FDEBUG(1) << spaces(8) << "processEvent................(" << id << ")\n";
}

void
art::EventProcessor::writeEvent()
{
  assert(eventPrincipal_);
  EventID const& id {eventPrincipal_->id()};
  // Precondition: The EventID does not correspond to a flush ID.
  assert(!id.isFlush());
  endPathExecutor_->writeEvent(*eventPrincipal_);
  FDEBUG(1) << spaces(8) << "writeEvent..................(" << id << ")\n";
  eventPrincipal_.reset();
}

bool
art::EventProcessor::shouldWeStop() const
{
  FDEBUG(1) << spaces(8) << "shouldWeStop\n";
  if (shouldWeStop_) { return true; }
  return endPathExecutor_->terminate();
}

bool
art::EventProcessor::setTriggerPathEnabled(std::string const& name, bool const enable)
{
  return schedule_->setTriggerPathEnabled(name, enable);
}

bool
art::EventProcessor::setEndPathModuleEnabled(std::string const& label, bool const enable)
{
  return endPathExecutor_->setEndPathModuleEnabled(label, enable);
}

void
art::EventProcessor::servicesActivate_(ServiceToken const st)
{
  servicesSentry_ = std::make_unique<ServiceRegistry::Operate>(st);
}

void
art::EventProcessor::servicesDeactivate_()
{
  servicesSentry_.reset();
}

void
art::EventProcessor::terminateAbnormally_()
try
{
  if (ServiceRegistry::instance().isAvailable<RandomNumberGenerator>()) {
    ServiceHandle<RandomNumberGenerator>{}->saveToFile_();
  }
}
catch (...)
{
}
