#ifndef art_Framework_Core_Frameworkfwd_h
#define art_Framework_Core_Frameworkfwd_h

// ======================================================================
//
// Forward declarations of types in the EDM.
//
// ======================================================================

#include "art/Persistency/Common/fwd.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "fhiclcpp/ParameterSet.h"

// ----------------------------------------------------------------------

namespace art {

  class ConfigurableInputSource;
  class CurrentProcessingContext;
  class EDAnalyzer;
  class EDFilter;
  class EDLooper;
  class EDProducer;
  class EndPathExecutor;
  class FileBlock;
  class GeneratedInputSource;
  class InputSource;
  struct InputSourceDescription;
  class OutputModule;
  struct OutputModuleDescription;
  class ProcessNameSelector;
  class ProductRegistryHelper;
  class Schedule;

  struct EventSummary;
  struct ModuleInPathSummary;
  struct PathSummary;
  struct TriggerReport;
  struct WorkerSummary;

} // namespace art

  // ======================================================================

#endif /* art_Framework_Core_Frameworkfwd_h */

// Local Variables:
// mode: c++
// End:
