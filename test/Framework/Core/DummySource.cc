/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include <stdexcept>
#include <memory>

#include "test/Framework/Core/DummySource.h"

namespace edm {
  DummySource::DummySource(ParameterSet const& pset,
				       InputSourceDescription const& desc) :
    GeneratedInputSource(pset, desc)
  { }

  DummySource::~DummySource() {
  }

  bool
  DummySource::produce(edm::Event &) {
    return true;
  }
}
