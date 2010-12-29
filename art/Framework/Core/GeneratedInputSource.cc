#include "art/Framework/Core/GeneratedInputSource.h"


namespace art {

  GeneratedInputSource::GeneratedInputSource( fhicl::ParameterSet const& pset
                                            , InputSourceDescription const& desc )
  : ConfigurableInputSource(pset, desc, false)
  { }

  GeneratedInputSource::~GeneratedInputSource()
  { }

}  // art
