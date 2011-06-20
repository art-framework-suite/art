#ifndef art_Framework_Core_Frameworkfwd_h
#define art_Framework_Core_Frameworkfwd_h

// ======================================================================
//
// Forward declarations of types in the EDM.
//
// ======================================================================

#include "art/Persistency/Common/fwd.h"
#include "art/Persistency/Provenance/ProvenanceFwd.h"
#include "fhiclcpp/ParameterSet.h"

// ----------------------------------------------------------------------

namespace art {

  class ConfigurableInputSource;
  class CurrentProcessingContext;
  class DelayedReader;
  class EDAnalyzer;
  class EDFilter;
  class EDLooper;
  class EDProducer;
  class FileBlock;
  class GeneratedInputSource;
  class InputSource;
  class InputSourceDescription;
  class OutputModule;
  class OutputModuleDescription;
  class ProcessNameSelector;
  class ProductRegistryHelper;
  class Schedule;

  struct EventSummary;
  struct PathSummary;
  struct TriggerReport;

}  // art

// The following are trivial enough so that the real headers can be included.
#include "art/Framework/Core/BranchActionType.h"

// ======================================================================

#endif /* art_Framework_Core_Frameworkfwd_h */

// Local Variables:
// mode: c++
// End:
