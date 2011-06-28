#include "art/Persistency/Provenance/TransientStreamer.h"

#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Persistency/Provenance/FileIndex.h"
#include "art/Persistency/Provenance/ProcessHistory.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/Transient.h"

void art::setProvenanceTransientStreamers()
{
  detail::SetTransientStreamer<Transient<BranchDescription::Transients> >();
  detail::SetTransientStreamer<Transient<FileIndex::Transients> >();
  detail::SetTransientStreamer<Transient<ProcessHistory::Transients> >();
  detail::SetTransientStreamer<Transient<ProductProvenance::Transients> >();
  detail::SetTransientStreamer<Transient<ProductRegistry::Transients> >();
}
