#ifndef art_Persistency_Provenance_FileIndex_h
#define art_Persistency_Provenance_FileIndex_h

////////////////////////////////////////////////////////////////////////
//
// This intra-file indexing system has known issues and will be going
// away at the earliest possible opportunity. In the meantime however,
// there are some things of which the user of this class should be
// aware:
//
// 1. This class is *not* intended for use outside ART. It should
//    probably be put into the detail namespace.
//
// 2. This class depends implicitly on an invalid value of the run,
//    subrun or event in an EventID sorting *before* a valid one. This
//    should be enforced in the comparison operations of the EventID and
//    related classes.
//
// 3. Due to user requirements, it *is* possible to findEventPosition()
//    using an EventID which is invalid in one particular way: the run
//    and event numbers are valid, but the subrun number is not. HOWEVER,
//    this only makes sense in an environment where the run number and
//    event number specify the event uniquely. No check is made that an
//    answer returned by findEventPosition() in these circumstances is
//    in any way unique.
//
////////////////////////////////////////////////////////////////////////

#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Provenance/RunID.h"
#include "art/Persistency/Provenance/SubRunID.h"
#include "art/Persistency/Provenance/Transient.h"
#include <cassert>
#include <iosfwd>
#include <vector>

namespace art {

   class FileIndex;

}

class art::FileIndex {

public:
   typedef long long EntryNumber_t;

   FileIndex();

   // use compiler-generated copy c'tor, copy assignment, and d'tor

   void addEntry(EventID const &eID, EntryNumber_t entry);

   void addEntryOnLoad(EventID const &eID, EntryNumber_t entry);

   enum EntryType {kRun, kSubRun, kEvent, kEnd};

   class Element {
   public:
      static EntryNumber_t const invalidEntry;
      Element() : eventID_(), entry_(invalidEntry) {
      }
      Element(EventID const &eID, EntryNumber_t entry = invalidEntry) :
         eventID_(eID), entry_(entry) {
      }
      EntryType getEntryType() const {
         return eventID_.isValid()?kEvent:(eventID_.subRunID().isValid()?kSubRun:kRun);
      }
      EventID eventID_;
      EntryNumber_t entry_;
   };

   typedef std::vector<Element>::const_iterator const_iterator;
   typedef std::vector<Element>::iterator iterator;

   void sortBy_Run_SubRun_Event();
   void sortBy_Run_SubRun_EventEntry();

   const_iterator
   findPosition(EventID const &eID) const;

   const_iterator
   findEventPosition(EventID const &eID, bool exact) const;

   const_iterator
   findSubRunPosition(SubRunID const &srID, bool exact) const;

   const_iterator
   findRunPosition(RunID const &rID, bool exact) const;

   const_iterator
   findSubRunOrRunPosition(SubRunID const &srID) const;

   bool
   containsEvent(EventID const &eID, bool exact) const {
      return findEventPosition(eID, exact) != entries_.end();
   }

   bool
   containsSubRun(SubRunID const &srID, bool exact) const {
      return findSubRunPosition(srID, exact) != entries_.end();
   }

   bool
   containsRun(RunID const &rID, bool exact) const {
      return findRunPosition(rID, exact) != entries_.end();
   }

   iterator begin() {return entries_.begin();}
   const_iterator cbegin() const { return entries_.begin();}

   iterator end() {return entries_.end();}
   const_iterator cend() const { return entries_.end();}

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

   const_iterator
   findEventForUnspecifiedSubRun(EventID const &eID, bool exact) const;

   std::vector<Element> entries_;
   mutable Transient<Transients> transients_;
};

namespace art {
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
  operator<< (std::ostream& os, FileIndex::Element const& el);

  std::ostream&
  operator<< (std::ostream& os, FileIndex const& fileIndex);
}

#endif /* art_Persistency_Provenance_FileIndex_h */

// Local Variables:
// mode: c++
// End:
