#ifndef Framework_Frameworkfwd_h
#define Framework_Frameworkfwd_h

// ======================================================================
//
// Forward declarations of types in the EDM.
//
// ======================================================================

#include "art/Persistency/Common/EDProductfwd.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "fhiclcpp/ParameterSet.h"

// ----------------------------------------------------------------------

namespace art {

  class ConfigurableInputSource;
  class CurrentProcessingContext;
  class DataViewImpl;
  class DelayedReader;
  class EDAnalyzer;
  class EDFilter;
  class EDLooper;
  class EDProducer;
  class Event;
  class EventPrincipal;
  class FileBlock;
  class GeneratedInputSource;
  class Group;
  class InputSource;
  class InputSourceDescription;
  class SubRun;
  class SubRunPrincipal;
  class NoDelayedReader;
  class OutputModule;
  class OutputModuleDescription;
  class Principal;
  class ProcessNameSelector;
  class ProductRegistryHelper;
  class Run;
  class RunPrincipal;
  class Schedule;
  class Selector;
  class SelectorBase;
  class TypeID;
  class UnscheduledHandler;

  struct EventSummary;
  struct PathSummary;
  struct TriggerReport;

}  // art

// The following are trivial enough so that the real headers can be included.
#include "art/Framework/Core/BranchActionType.h"

// ======================================================================

#endif
