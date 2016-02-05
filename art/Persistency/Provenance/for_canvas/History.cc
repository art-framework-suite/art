#include "canvas/Persistency/Provenance/History.h"

namespace art
{

  History::size_type
  History::size() const {
    return eventSelections_.size();
  }

  void
  History::addEventSelectionEntry(EventSelectionID const& eventSelection) {
    eventSelections_.push_back(eventSelection);
  }

  void
  History::addBranchListIndexEntry(BranchListIndex const& branchListIndex) {
    branchListIndexes_.push_back(branchListIndex);
  }

  EventSelectionID const&
  History::getEventSelectionID(History::size_type i) const {
    return eventSelections_[i];
  }

  EventSelectionIDVector const&
  History::eventSelectionIDs() const {
    return eventSelections_;
  }

}
