#ifndef art_Framework_Core_fwd_h
#define art_Framework_Core_fwd_h

#include "art/Persistency/Common/fwd.h"
#include "canvas/Persistency/Provenance/fwd.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class EDAnalyzer;
  class EDFilter;
  class EDProducer;
  class EndPathExecutor;
  class FileBlock;
  class GroupSelector;
  class InputSource;
  struct InputSourceDescription;
  class OutputModule;
  class OutputWorker;
  class PathsInfo;
  class PathManager;
  class ProcessNameSelector;
  class ProcessingLimits;
  class ProducesCollector;
  class ProductRegistryHelper;
  class ReplicatedAnalyzer;
  class ReplicatedFilter;
  class ReplicatedProducer;
  class Schedule;
  class SharedAnalyzer;
  class SharedFilter;
  class SharedProducer;
  class UpdateOutputCallbacks;
} // namespace art

#endif /* art_Framework_Core_fwd_h */

// Local Variables:
// mode: c++
// End:
