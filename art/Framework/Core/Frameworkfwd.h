#ifndef art_Framework_Core_Frameworkfwd_h
#define art_Framework_Core_Frameworkfwd_h

#include "art/Persistency/Common/fwd.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class EDAnalyzer;
  class EDFilter;
  class EDProducer;
  class ReplicatedAnalyzer;
  class ReplicatedFilter;
  class ReplicatedProducer;
  class SharedAnalyzer;
  class SharedFilter;
  class SharedProducer;
  class EndPathExecutor;
  class FileBlock;
  class InputSource;
  class OutputModule;
  class ProcessNameSelector;
  class ProductRegistryHelper;
  class Schedule;
} // namespace art

#endif /* art_Framework_Core_Frameworkfwd_h */

// Local Variables:
// mode: c++
// End:
