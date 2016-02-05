#include "canvas/Persistency/Provenance/TransientStreamer.h"

#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/FileIndex.h"
#include "canvas/Persistency/Provenance/ProcessHistory.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"
#include "canvas/Persistency/Provenance/Transient.h"

void art::setProvenanceTransientStreamers()
{
  detail::SetTransientStreamer<Transient<BranchDescription::Transients> >();
  detail::SetTransientStreamer<Transient<FileIndex::Transients> >();
  detail::SetTransientStreamer<Transient<ProcessHistory::Transients> >();
  detail::SetTransientStreamer<Transient<ProductProvenance::Transients> >();
}
