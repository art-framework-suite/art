#ifndef DataFormats_Provenance_ProcessConfigurationID_h
#define DataFormats_Provenance_ProcessConfigurationID_h

#include "art/Persistency/Provenance/HashedTypes.h"
#include "art/Persistency/Provenance/Hash.h"

namespace art
{
  typedef Hash<ProcessConfigurationType> ProcessConfigurationID;
}


#endif
