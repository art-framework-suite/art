/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include <stdexcept>
#include <memory>

#include "test/Framework/Core/DummySource.h"

namespace art {
   DummySource::DummySource(fhicl::ParameterSet const& pset,
				       InputSourceDescription const& desc) :
    GeneratedInputSource(pset, desc)
  { }

  DummySource::~DummySource() {
  }

  bool
  DummySource::produce(art::Event &) {
    return true;
  }
}
