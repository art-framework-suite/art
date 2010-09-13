#include "art/Framework/Core/GeneratedInputSource.h"


namespace edm {

  GeneratedInputSource::GeneratedInputSource( fhicl::ParameterSet const& pset
                                            , InputSourceDescription const& desc )
  : ConfigurableInputSource(pset, desc, false)
  { }

  GeneratedInputSource::~GeneratedInputSource()
  { }

}  // namespace edm
