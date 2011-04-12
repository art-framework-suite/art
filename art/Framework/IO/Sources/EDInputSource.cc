#include "art/Framework/IO/Sources/EDInputSource.h"

#include "messagefacility/MessageLogger/MessageLogger.h"


namespace art {

  EDInputSource::EDInputSource(fhicl::ParameterSet const& pset,
                               InputSourceDescription const& desc) :
      DecrepitRelicInputSourceImplementation(pset, desc),
      catalog_(pset)
  { }

  EDInputSource::~EDInputSource() { }

}  // art
