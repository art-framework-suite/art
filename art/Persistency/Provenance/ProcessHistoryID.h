#ifndef DataFormats_Provenance_ProcessHistoryID_h
#define DataFormats_Provenance_ProcessHistoryID_h

#include "art/Persistency/Provenance/HashedTypes.h"
#include "art/Persistency/Provenance/Hash.h"

namespace art
{
  typedef Hash<ProcessHistoryType> ProcessHistoryID;
}


#endif
