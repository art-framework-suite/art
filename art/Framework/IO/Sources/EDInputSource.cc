#include "art/Framework/IO/Sources/EDInputSource.h"

#include "MessageFacility/MessageLogger.h"


namespace edm {

  EDInputSource::EDInputSource(fhicl::ParameterSet const& pset,
                               InputSourceDescription const& desc) :
      InputSource(pset, desc),
      poolCatalog_(),
      catalog_(pset, poolCatalog_),
      secondaryCatalog_(pset, poolCatalog_, std::string("secondaryFileNames"), true) {}

  EDInputSource::~EDInputSource() { }

  void
  EDInputSource::setRun(RunNumber_t) {
      mf::LogWarning("IllegalCall")
        << "EDInputSource::setRun()\n"
           "Run number cannot be modified for an EDInputSource\n";
  }

  void
  EDInputSource::setSubRun(SubRunNumber_t) {
      mf::LogWarning("IllegalCall")
        << "EDInputSource::setSubRun()\n"
           "SubRun ID cannot be modified for an EDInputSource\n";
  }

}  // namespace edm
