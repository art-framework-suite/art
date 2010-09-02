/*----------------------------------------------------------------------

----------------------------------------------------------------------*/

#include "art/Framework/IO/Sources/EDInputSource.h"
#include "art/MessageLogger/MessageLogger.h"

namespace edm {

  EDInputSource::EDInputSource(ParameterSet const& pset,
				       InputSourceDescription const& desc) :
      InputSource(pset, desc),
      poolCatalog_(),
      catalog_(pset, poolCatalog_),
      secondaryCatalog_(pset, poolCatalog_, std::string("secondaryFileNames"), true) {}

  EDInputSource::~EDInputSource() {
  }

  void
  EDInputSource::setRun(RunNumber_t) {
      LogWarning("IllegalCall")
        << "EDInputSource::setRun()\n"
        << "Run number cannot be modified for an EDInputSource\n";
  }

  void
  EDInputSource::setLumi(SubRunNumber_t) {
      LogWarning("IllegalCall")
        << "EDInputSource::setLumi()\n"
        << "SubRun ID cannot be modified for an EDInputSource\n";
  }
}
