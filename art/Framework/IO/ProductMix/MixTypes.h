#ifndef art_Framework_IO_ProductMix_MixTypes_h
#define art_Framework_IO_ProductMix_MixTypes_h

#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/FileIndex.h"

#include <functional>
#include <map>
#include <memory>
#include <vector>

namespace art {
  class EventAuxiliary;
  class EventID;
  class PtrRemapper;

  template <typename PROD, typename OPROD = PROD>
  using MixFunc = std::function<
    bool(std::vector<PROD const*> const&, OPROD&, PtrRemapper const&)>;

  using SpecProdList = std::vector<std::shared_ptr<EDProduct const>>;

  using EventIDIndex = std::map<FileIndex::EntryNumber_t, EventID>;
  using SubRunIDIndex = std::map<SubRunID, FileIndex::EntryNumber_t>;
  using RunIDIndex = std::map<SubRunID, FileIndex::EntryNumber_t>;
  using EventIDSequence = std::vector<EventID>;
  using EntryNumberSequence = std::vector<FileIndex::EntryNumber_t>;
  using EventAuxiliarySequence = std::vector<EventAuxiliary>;
} // namespace art

#endif /* art_Framework_IO_ProductMix_MixTypes_h */

// Local Variables:
// mode: c++
// End:
