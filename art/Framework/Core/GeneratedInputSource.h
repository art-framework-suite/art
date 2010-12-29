#ifndef Framework_GeneratedInputSource_h
#define Framework_GeneratedInputSource_h

// ======================================================================
//
// GeneratedInputSource
//
// ======================================================================

#include "art/Framework/Core/ConfigurableInputSource.h"
#include "fhiclcpp/ParameterSet.h"

// ----------------------------------------------------------------------

namespace art {

  class GeneratedInputSource
    : public ConfigurableInputSource
  {
  public:
    GeneratedInputSource( fhicl::ParameterSet    const & pset
                        , InputSourceDescription const & desc );
    virtual ~GeneratedInputSource();

  };  // GeneratedInputSource

}  // art

// ======================================================================

#endif
