#include "art/Framework/Core/GeneratedInputSource.h"

namespace edm {
    GeneratedInputSource::GeneratedInputSource(ParameterSet const& pset,
        InputSourceDescription const& desc) :
        ConfigurableInputSource(pset, desc, false) {
    }
    GeneratedInputSource::~GeneratedInputSource() {}
}
