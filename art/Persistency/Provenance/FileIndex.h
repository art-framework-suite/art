#ifndef DataFormats_Provenance_FileIndex_h
#define DataFormats_Provenance_FileIndex_h

/*----------------------------------------------------------------------

FileIndex.h



----------------------------------------------------------------------*/

#include <vector>
#include <cassert>
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/Transient.h"

#include <iosfwd>

namespace art {

  class FileIndex {

    public:
      typedef long long EntryNumber_t;

      FileIndex();
      ~FileIndex() {}

      void addEntry(RunNumber_t run, SubRunNumber_t subRun, EventNumber_t event, EntryNumber_t entry);

      enum EntryType {kRun, kSubRun, kEvent, kEnd};

      class Element {
        public:
	  static EntryNumber_t const invalidEntry = -1LL;
          Element() : run_(0U), subRun_(0U), event_(0U), entry_(invalidEntry) {
	  }
          Element(RunNumber_t run, SubRunNumber_t subRun, EventNumber_t event, long long entry) :
            run_(run), subRun_(subRun),
          event_(event), entry_(entry) {
	    assert(subRun_ != 0U || event_ == 0U);
	  }
          Element(RunNumber_t run, SubRunNumber_t subRun, EventNumber_t event) :
            run_(run), subRun_(subRun), event_(event), entry_(invalidEntry) {}
          EntryType getEntryType() const {
	    return subRun_ == 0U ? kRun : (event_ == 0U ? kSubRun : kEvent);
          }
          RunNumber_t run_;
          SubRunNumber_t subRun_;
          EventNumber_t event_;
          EntryNumber_t entry_;
      };

      typedef std::vector<Element>::const_iterator const_iterator;

      void sortBy_Run_SubRun_Event();
      void sortBy_Run_SubRun_EventEntry();

      const_iterator
      findPosition(RunNumber_t run, SubRunNumber_t subRun = 0U, EventNumber_t event = 0U) const;

      const_iterator
      findEventPosition(RunNumber_t run, SubRunNumber_t subRun, EventNumber_t event, bool exact) const;

      const_iterator
      findSubRunPosition(RunNumber_t run, SubRunNumber_t subRun, bool exact) const;

      const_iterator
      findRunPosition(RunNumber_t run, bool exact) const;

      const_iterator
      findSubRunOrRunPosition(RunNumber_t run, SubRunNumber_t subRun) const;

      bool
      containsEvent(RunNumber_t run, SubRunNumber_t subRun, EventNumber_t event, bool exact) const {
	return findEventPosition(run, subRun, event, exact) != entries_.end();
      }

      bool
      containsSubRun(RunNumber_t run, SubRunNumber_t subRun, bool exact) const {
        return findSubRunPosition(run, subRun, exact) != entries_.end();
      }

      bool
      containsRun(RunNumber_t run, bool exact) const {
        return findRunPosition(run, exact) != entries_.end();
      }

      const_iterator begin() const {return entries_.begin();}

      const_iterator end() const {return entries_.end();}

      std::vector<Element>::size_type size() const {return entries_.size();}

      bool empty() const {return entries_.empty();}

      bool allEventsInEntryOrder() const;

      bool eventsUniqueAndOrdered() const;

      enum SortState { kNotSorted, kSorted_Run_SubRun_Event, kSorted_Run_SubRun_EventEntry};

      struct Transients {
	Transients();
	bool allInEntryOrder_;
	bool resultCached_;
	SortState sortState_;
      };

    private:

      bool& allInEntryOrder() const {return transients_.get().allInEntryOrder_;}
      bool& resultCached() const {return transients_.get().resultCached_;}
      SortState& sortState() const {return transients_.get().sortState_;}

      std::vector<Element> entries_;
      mutable Transient<Transients> transients_;
  };

  bool operator<(FileIndex::Element const& lh, FileIndex::Element const& rh);

  inline
  bool operator>(FileIndex::Element const& lh, FileIndex::Element const& rh) {return rh < lh;}

  inline
  bool operator>=(FileIndex::Element const& lh, FileIndex::Element const& rh) {return !(lh < rh);}

  inline
  bool operator<=(FileIndex::Element const& lh, FileIndex::Element const& rh) {return !(rh < lh);}

  inline
  bool operator==(FileIndex::Element const& lh, FileIndex::Element const& rh) {return !(lh < rh || rh < lh);}

  inline
  bool operator!=(FileIndex::Element const& lh, FileIndex::Element const& rh) {return lh < rh || rh < lh;}

  class Compare_Run_SubRun_EventEntry {
  public:
    bool operator()(FileIndex::Element const& lh, FileIndex::Element const& rh);
  };

  std::ostream&
  operator<< (std::ostream& os, FileIndex const& fileIndex);
}

#endif
