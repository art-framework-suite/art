/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/Modules/EmptySource.h"

namespace edm {
  EmptySource::EmptySource(ParameterSet const& pset,
				       InputSourceDescription const& desc) :
    GeneratedInputSource(pset, desc)
  { }

  EmptySource::~EmptySource() {
  }

  bool
  EmptySource::produce(edm::Event &) {
    return true;
  }
}
