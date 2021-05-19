#ifndef art_Framework_Core_Frameworkfwd_h
#define art_Framework_Core_Frameworkfwd_h

#include "art/Persistency/Common/fwd.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class EDAnalyzer;
  class EDFilter;
  class EDProducer;
  class EndPathExecutor;
  class FileBlock;
  class GroupSelector;
  class InputSource;
  class OutputModule;
  class OutputWorker;
  class PathsInfo;
  class PathManager;
  class ProcessNameSelector;
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

#endif /* art_Framework_Core_Frameworkfwd_h */

// Local Variables:
// mode: c++
// End:
