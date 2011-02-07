#include "art/Framework/IO/Sources/EDInputSource.h"

#include "messagefacility/MessageLogger/MessageLogger.h"


namespace art {

  EDInputSource::EDInputSource(fhicl::ParameterSet const& pset,
                               InputSourceDescription const& desc) :
      InputSource(pset, desc),
      catalog_(pset),
      secondaryCatalog_(pset, std::string("secondaryFileNames"), true) {}

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

}  // art
