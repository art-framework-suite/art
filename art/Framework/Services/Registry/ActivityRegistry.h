#ifndef art_Framework_Services_Registry_ActivityRegistry_h
#define art_Framework_Services_Registry_ActivityRegistry_h

////////////////////////////////////////////////////////////////////////
// ActivityRegistry
//
// Registry holding the signals to which services may subscribe.
//
// Services can connect to the signals distributed by the
// ActivityRegistry in order to monitor the activity of the application.
//
// Signals are either global or local (per-schedule). Register a
// watchpoint by calling the watch() function of the appropriate signal.
//
//  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription
//  const&)> sPreModuleBeginJob;
//
// describes a watchpoint whose callable objects should have void return
// type and take a single argument of type ModuleDescription const&.
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Services/Registry/GlobalSignal.h"
#include "art/Framework/Services/Registry/detail/SignalResponseType.h"

#include <functional>
#include <string>
#include <vector>

namespace art {
  // Explicit declarations here rather than including fwd.h to avoid
  // confusing our dependency checker into thinking there's a link
  // dependency.
  class Event;
  class HLTPathStatus;
  class InputSource;
  class ModuleDescription;
  class OutputFileInfo;
  class Run;
  class RunID;
  class SubRun;
  class SubRunID;
  class Timestamp;
  class Worker;

  class ActivityRegistry;

  class EventProcessor;
  class Schedule;
} // art

class art::ActivityRegistry {
public:
  ActivityRegistry() = default;
  ActivityRegistry(ActivityRegistry const&) = delete;
  ActivityRegistry& operator=(ActivityRegistry const&) = delete;

  // ---------- signals ------------------------------------
  // Signal is emitted after all modules have had their beginJob called
  GlobalSignal<detail::SignalResponseType::FIFO, void()> sPostBeginJob;

  // Signal is emitted after all modules have had their endJob called
  GlobalSignal<detail::SignalResponseType::LIFO, void()> sPostEndJob;

  // Signal is emitted if event processing or end-of-job
  // processing fails with an uncaught exception.
  GlobalSignal<detail::SignalResponseType::LIFO, void()> sJobFailure;

  // Signal is emitted after the source's constructor is called
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostSourceConstruction;

  // Signal is emitted before the source starts creating an Event
  GlobalSignal<detail::SignalResponseType::FIFO, void()> sPreSourceEvent;

  // Signal is emitted after the source starts creating an Event
  GlobalSignal<detail::SignalResponseType::LIFO, void(Event const&)>
    sPostSourceEvent;

  // Signal is emitted before the source starts creating a SubRun
  GlobalSignal<detail::SignalResponseType::FIFO, void()> sPreSourceSubRun;

  // Signal is emitted after the source starts creating a SubRun
  GlobalSignal<detail::SignalResponseType::LIFO, void(SubRun const&)>
    sPostSourceSubRun;

  // Signal is emitted before the source starts creating a Run
  GlobalSignal<detail::SignalResponseType::FIFO, void()> sPreSourceRun;

  // Signal is emitted after the source starts creating a Run
  GlobalSignal<detail::SignalResponseType::LIFO, void(Run const&)>
    sPostSourceRun;

  // Signal is emitted before the source opens a file
  GlobalSignal<detail::SignalResponseType::FIFO, void()> sPreOpenFile;

  // Signal is emitted after the source opens a file
  GlobalSignal<detail::SignalResponseType::LIFO, void(std::string const&)>
    sPostOpenFile;

  // Signal is emitted before the source closes a file.
  GlobalSignal<detail::SignalResponseType::FIFO, void()> sPreCloseFile;

  // Signal is emitted after the source closes a file
  GlobalSignal<detail::SignalResponseType::LIFO, void()> sPostCloseFile;

  // Signal is emitted just after the output opens a file (provides
  // module label).
  GlobalSignal<detail::SignalResponseType::LIFO, void(std::string const&)>
    sPostOpenOutputFile;

  // Signal is emitted just before an output will close a file (provides
  // module label).
  GlobalSignal<detail::SignalResponseType::FIFO, void(std::string const&)>
    sPreCloseOutputFile;

  // Signal is emitted after an output has closed a file (provides
  // module label and file name).
  GlobalSignal<detail::SignalResponseType::LIFO, void(OutputFileInfo const&)>
    sPostCloseOutputFile;

  // Signal is emitted after the Event has been created by the
  // InputSource but before any modules have seen the Event
  GlobalSignal<detail::SignalResponseType::FIFO, void(Event const&)>
    sPreProcessEvent;

  // Signal is emitted after all modules have finished processing the
  // Event
  GlobalSignal<detail::SignalResponseType::LIFO, void(Event const&)>
    sPostProcessEvent;

  // Signal is emitted after the event has been processed, but before
  // the event has been written.
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreWriteEvent;

  // Signal is emitted after the event has been written.
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostWriteEvent;

  // Signal is emitted after the Run has been created by the InputSource
  // but before any modules have seen the Run
  GlobalSignal<detail::SignalResponseType::FIFO, void(Run const&)> sPreBeginRun;

  // Signal is emitted after all modules have finished processing the
  // beginRun
  GlobalSignal<detail::SignalResponseType::LIFO, void(Run const&)>
    sPostBeginRun;

  // Signal is emitted before the endRun is processed
  GlobalSignal<detail::SignalResponseType::FIFO,
               void(RunID const&, Timestamp const&)>
    sPreEndRun;

  // Signal is emitted after all modules have finished processing the
  // Run
  GlobalSignal<detail::SignalResponseType::LIFO, void(Run const&)> sPostEndRun;

  // Signal is emitted after the SubRun has been created by the
  // InputSource but before any modules have seen the SubRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(SubRun const&)>
    sPreBeginSubRun;

  // Signal is emitted after all modules have finished processing the
  // beginSubRun
  GlobalSignal<detail::SignalResponseType::LIFO, void(SubRun const&)>
    sPostBeginSubRun;

  // Signal is emitted before the endSubRun is processed
  GlobalSignal<detail::SignalResponseType::FIFO,
               void(SubRunID const&, Timestamp const&)>
    sPreEndSubRun;

  // Signal is emitted after all modules have finished processing the
  // SubRun
  GlobalSignal<detail::SignalResponseType::LIFO, void(SubRun const&)>
    sPostEndSubRun;

  // Signal is emitted before starting to process a Path for an event
  GlobalSignal<detail::SignalResponseType::FIFO, void(std::string const&)>
    sPreProcessPath;

  // Signal is emitted after all modules have finished for the Path for
  // an event
  GlobalSignal<detail::SignalResponseType::LIFO,
               void(std::string const&, HLTPathStatus const&)>
    sPostProcessPath;

  // Signal is emitted before starting to process a Path for beginRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(std::string const&)>
    sPrePathBeginRun;

  // Signal is emitted after all modules have finished for the Path for beginRun
  GlobalSignal<detail::SignalResponseType::LIFO,
               void(std::string const&, HLTPathStatus const&)>
    sPostPathBeginRun;

  // Signal is emitted before starting to process a Path for endRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(std::string const&)>
    sPrePathEndRun;

  // Signal is emitted after all modules have finished for the Path for endRun
  GlobalSignal<detail::SignalResponseType::LIFO,
               void(std::string const&, HLTPathStatus const&)>
    sPostPathEndRun;

  // Signal is emitted before starting to process a Path for beginSubRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(std::string const&)>
    sPrePathBeginSubRun;

  // Signal is emitted after all modules have finished for the Path for
  // beginSubRun
  GlobalSignal<detail::SignalResponseType::LIFO,
               void(std::string const&, HLTPathStatus const&)>
    sPostPathBeginSubRun;

  // Signal is emitted before starting to process a Path for endRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(std::string const&)>
    sPrePathEndSubRun;

  // Signal is emitted after all modules have finished for the Path for endRun
  GlobalSignal<detail::SignalResponseType::LIFO,
               void(std::string const&, HLTPathStatus const&)>
    sPostPathEndSubRun;

  // Signal is emitted before the module is constructed
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleConstruction;

  // Signal is emitted after the module was construction
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleConstruction;

  // Signal is emitted before module does respondToOpenInputFile
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleRespondToOpenInputFile;

  // Signal is emitted after module has done respondToOpenInputFile
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleRespondToOpenInputFile;

  // Signal is emitted before module does respondToCloseInputFile
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleRespondToCloseInputFile;

  // Signal is emitted after module has done respondToCloseInputFile
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleRespondToCloseInputFile;

  // Signal is emitted before module does respondToOpenOutputFiles
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleRespondToOpenOutputFiles;

  // Signal is emitted after module has done respondToOpenOutputFiles
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleRespondToOpenOutputFiles;

  // Signal is emitted before module does respondToCloseOutputFiles
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleRespondToCloseOutputFiles;

  // Signal is emitted after module has done respondToCloseOutputFiles
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleRespondToCloseOutputFiles;

  // Signal is emitted after beginJob
  GlobalSignal<detail::SignalResponseType::LIFO,
               void(InputSource*, std::vector<Worker*> const&)>
    sPostBeginJobWorkers;

  // Signal is emitted before the module does beginJob
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleBeginJob;

  // Signal is emitted after the module had done beginJob
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleBeginJob;

  // Signal is emitted before the module does endJob
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleEndJob;

  // Signal is emitted after the module had done endJob
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleEndJob;

  // Signal is emitted before the module starts processing the Event
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModule;

  // Signal is emitted after the module finished processing the Event
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModule;

  // Signal is emitted before the module starts processing beginRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleBeginRun;

  // Signal is emitted after the module finished processing beginRun
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleBeginRun;

  // Signal is emitted before the module starts processing endRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleEndRun;

  // Signal is emitted after the module finished processing endRun
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleEndRun;

  // Signal is emitted before the module starts processing beginSubRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleBeginSubRun;

  // Signal is emitted after the module finished processing beginSubRun
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleBeginSubRun;

  // Signal is emitted before the module starts processing endSubRun
  GlobalSignal<detail::SignalResponseType::FIFO, void(ModuleDescription const&)>
    sPreModuleEndSubRun;

  // Signal is emitted after the module finished processing endSubRun
  GlobalSignal<detail::SignalResponseType::LIFO, void(ModuleDescription const&)>
    sPostModuleEndSubRun;

}; // ActivityRegistry

#endif /* art_Framework_Services_Registry_ActivityRegistry_h */

// Local Variables:
// mode: c++
// End:
