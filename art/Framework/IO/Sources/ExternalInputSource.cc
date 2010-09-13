#include "art/Framework/IO/Sources/ExternalInputSource.h"

using fhicl::ParameterSet;

namespace edm {

  ExternalInputSource::ExternalInputSource( ParameterSet const& pset
                                          , InputSourceDescription const& desc
                                          , bool realData )
  : ConfigurableInputSource( pset, desc, realData )
  , poolCatalog_           ( )
  , catalog_               ( pset, poolCatalog_ )
  { }

  ExternalInputSource::~ExternalInputSource()
  { }

}  // namespace edm
