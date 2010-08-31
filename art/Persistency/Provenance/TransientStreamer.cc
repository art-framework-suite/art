#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/Transient.h"
#include "art/Persistency/Provenance/TransientStreamer.h"

#include "art/Persistency/Provenance/EventEntryDescription.h" // backward compatibility
#include "art/Persistency/Provenance/EventEntryInfo.h" // backward compatibility

namespace edm {
  void setTransientStreamers() {
    SetTransientStreamer<Transient<BranchDescription::Transients> >();
    SetTransientStreamer<Transient<ProductProvenance::Transients> >();
    SetTransientStreamer<Transient<FileIndex::Transients> >();
    SetTransientStreamer<Transient<ProcessHistory::Transients> >();
    SetTransientStreamer<Transient<ProductRegistry::Transients> >();
    SetTransientStreamer<Transient<EventEntryDescription::Transients> >(); // backward compatibility
    SetTransientStreamer<Transient<EventEntryInfo::Transients> >(); // backward compatibility
  }
}
