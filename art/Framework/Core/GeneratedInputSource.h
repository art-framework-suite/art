#ifndef Framework_GeneratedInputSource_h
#define Framework_GeneratedInputSource_h

/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include <memory>

#include "art/Framework/Core/ConfigurableInputSource.h"

namespace edm {
  class GeneratedInputSource : public ConfigurableInputSource {
  public:
    explicit GeneratedInputSource(ParameterSet const& pset, InputSourceDescription const& desc);
    virtual ~GeneratedInputSource();

  };
}
#endif
