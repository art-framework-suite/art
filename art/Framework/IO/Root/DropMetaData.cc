#include "art/Framework/IO/Root/DropMetaData.h"
#include "canvas/Utilities/Exception.h"

using art::DropMetaData;

DropMetaData::DropMetaData(enum_t const e)
  : value_{e}
{}

DropMetaData::DropMetaData(std::string const& config)
  : value_{strToValue_(config)}
{}

DropMetaData::enum_t
DropMetaData::strToValue_(std::string const& dropMetaData)
{
  enum_t result {DropNone};
  if (dropMetaData == "NONE") {
    result = DropNone;
  }
  else if (dropMetaData == "PRIOR") {
    result = DropPrior;
  }
  else if (dropMetaData == "ALL") {
    result = DropAll;
  }
  else {
    throw art::Exception(errors::Configuration,
                         "Illegal dropMetaData parameter value: ")
      << dropMetaData << ".\n"
      << "Legal values are 'NONE', 'PRIOR', and 'ALL'.\n";
  }
  return result;
}
