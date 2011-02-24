#include "art/Persistency/Provenance/FileIndex.h"

#include "cetlib/container_algorithms.h"
#include <algorithm>
#include <iomanip>
#include <ostream>

using namespace cet;
using namespace std;


namespace {
   bool subRunUnspecified(art::EventID const &eID) {
      // This is nasty, principally because we don't want to be
      // encouraging too many people to do this. Basically, we're
      // checking whether the only reason an EventID is invalid is
      // because its subRun number is invalid.
      return (!eID.isValid()) &&
         eID.runID().isValid() &&
         art::EventID(art::SubRunID::firstSubRun(),
                      eID.event()).isValid();
   }
}

namespace art {

   art::FileIndex::EntryNumber_t const art::FileIndex::Element::invalidEntry = -1;

   FileIndex::FileIndex() : entries_(), transients_() {}

   // The default value for sortState_ reflects the fact that
   // the index is always sorted using Run, SubRun, and Event
   // number by the RootOutput before being written out.
   // In the other case when we create a new FileIndex, the
   // vector is empty, which is consistent with it having been
   // sorted.

   FileIndex::Transients::Transients() : allInEntryOrder_(false), resultCached_(false), sortState_(kSorted_Run_SubRun_Event) {}

   void
   FileIndex::addEntry(EventID const &eID, EntryNumber_t entry) {
      entries_.push_back(FileIndex::Element(eID, entry));
      resultCached() = false;
      sortState() = kNotSorted;
   }

   void FileIndex::sortBy_Run_SubRun_Event() {
      stable_sort_all(entries_);
      resultCached() = false;
      sortState() = kSorted_Run_SubRun_Event;
   }

   void FileIndex::sortBy_Run_SubRun_EventEntry() {
      stable_sort_all(entries_, Compare_Run_SubRun_EventEntry());
      resultCached() = false;
      sortState() = kSorted_Run_SubRun_EventEntry;
   }

   bool FileIndex::allEventsInEntryOrder() const {
      if (!resultCached()) {
         resultCached() = true;
         EntryNumber_t maxEntry = Element::invalidEntry;
         for (vector<FileIndex::Element>::const_iterator it = entries_.begin(), itEnd = entries_.end(); it != itEnd; ++it) {
            if (it->getEntryType() == kEvent) {
               if (it->entry_ < maxEntry) {
                  allInEntryOrder() = false;
                  return false;
               }
               maxEntry = it->entry_;
            }
         }
         allInEntryOrder() = true;
         return true;
      }
      return allInEntryOrder();
   }

   bool FileIndex::eventsUniqueAndOrdered() const {

      const_iterator it = begin();
      const_iterator itEnd = end();

      // Set up the iterators to point to first two events
      // (In the trivial case where there is zero or one event,
      // the set must be unique and ordered)

      if (it == itEnd) return true;

      // Step to first event
      while (it->getEntryType() != kEvent) {
         ++it;
         if (it == itEnd) return true;
      }
      const_iterator itPrevious = it;

      // Step to second event
      ++it;
      if (it == itEnd) return true;
      while (it->getEntryType() != kEvent) {
         ++it;
         if (it == itEnd) return true;
      }

      for ( ; it != itEnd; ++it) {
         if (it->getEntryType() == kEvent) {
            if (it->eventID_ <= itPrevious->eventID_) return false;
            itPrevious = it;
         }
      }
      return true; // finished and found no duplicates
   }

   FileIndex::const_iterator
   FileIndex::findPosition(EventID const &eID) const {

      assert(sortState() == kSorted_Run_SubRun_Event);

      Element el(eID);
      return lower_bound_all(entries_, el);
   }

   FileIndex::const_iterator
   FileIndex::findEventPosition(EventID const &eID, bool exact) const {

      assert(sortState() == kSorted_Run_SubRun_Event);

      if (subRunUnspecified(eID)) {
         return findEventForUnspecifiedSubRun(eID, exact);
      }

      const_iterator it = findPosition(eID);
      const_iterator itEnd = entries_.end();
      while (it != itEnd && it->getEntryType() != FileIndex::kEvent) {
         ++it;
      }
      if (it == itEnd) return itEnd;
      if (exact && (*it != eID)) return itEnd;
      return it;
   }

   FileIndex::const_iterator
   FileIndex::findSubRunPosition(SubRunID const &srID, bool exact) const {
      assert(sortState() != kNotSorted);
      const_iterator it;
      if (sortState() == kSorted_Run_SubRun_EventEntry) {
         Element el(EventID::invalidEvent(srID));
         it = lower_bound_all(entries_, el, Compare_Run_SubRun_EventEntry());
      }
      else {
         it = findPosition(EventID::invalidEvent(srID));
      }
      const_iterator itEnd = entries_.end();
      while (it != itEnd && it->getEntryType() != FileIndex::kSubRun) {
         ++it;
      }
      if (it == itEnd) return itEnd;
      if (exact && (it->eventID_.subRunID() != srID)) return itEnd;
      return it;
   }

   FileIndex::const_iterator
   FileIndex::findRunPosition(RunID const &rID, bool exact) const {
      assert(sortState() != kNotSorted);
      const_iterator it;
      if (sortState() == kSorted_Run_SubRun_EventEntry) {
         Element el(EventID::invalidEvent(rID));
         it = lower_bound_all(entries_, el, Compare_Run_SubRun_EventEntry());
      }
      else {
         it = findPosition(EventID::invalidEvent(rID));
      }
      const_iterator itEnd = entries_.end();
      while (it != itEnd && it->getEntryType() != FileIndex::kRun) {
         ++it;
      }
      if (it == itEnd) return itEnd;
      if (exact && (it->eventID_.runID() != rID)) return itEnd;
      return it;
   }

   FileIndex::const_iterator
   FileIndex::findSubRunOrRunPosition(SubRunID const &srID) const {
      assert(sortState() != kNotSorted);
      const_iterator it;
      if (sortState() == kSorted_Run_SubRun_EventEntry) {
         Element el(EventID::invalidEvent(srID));
         it = lower_bound_all(entries_, el, Compare_Run_SubRun_EventEntry());
      }
      else {
         it = findPosition(EventID::invalidEvent(srID));
      }
      const_iterator itEnd = entries_.end();
      while (it != itEnd && it->getEntryType() != FileIndex::kSubRun && it->getEntryType() != FileIndex::kRun) {
         ++it;
      }
      return it;
   }

   bool operator<(FileIndex::Element const& lh, FileIndex::Element const& rh) {
      bool result = lh.eventID_ < rh.eventID_;
      return result;
   }

   bool Compare_Run_SubRun_EventEntry::operator()(FileIndex::Element const& lh, FileIndex::Element const& rh)
   {
      if (lh.eventID_.subRunID() == rh.eventID_.subRunID()) {
         if ((!lh.eventID_.isValid()) && (!rh.eventID_.isValid())) {
            return false;
         } else if (!lh.eventID_.isValid()) {
            return true;
         } else if (!rh.eventID_.isValid()) {
            return false;
         } else {
            return lh.entry_ < rh.entry_;
         }
      } else {
         return lh.eventID_.subRunID() < rh.eventID_.subRunID();
      }
   }

   ostream &operator<<(ostream &os, FileIndex::Element const &el) {
      os << el.eventID_ << ": entry# " << el.entry_;
      return os;
   }

   ostream&
   operator<< (ostream& os, FileIndex const& fileIndex) {

      os << "\nPrinting FileIndex contents.  This includes a list of all Runs, SubRuns\n"
         << "and Events stored in the root file.\n\n";
      os << setw(15) << "Run"
         << setw(15) << "SubRun"
         << setw(15) << "Event"
         << setw(15) << "TTree Entry"
         << "\n";
      for (vector<FileIndex::Element>::const_iterator
              it = fileIndex.begin(),
              itEnd = fileIndex.end();
           it != itEnd;
           ++it) {
         if (it->getEntryType() == FileIndex::kEvent) {
            os << setw(15) << it->eventID_.run()
               << setw(15) << it ->eventID_.subRun()
               << setw(15) << it->eventID_.event()
               << setw(15) << it->entry_
               << "\n";
         }
         else if (it->getEntryType() == FileIndex::kSubRun) {
            os << setw(15) << it->eventID_.run()
               << setw(15) << it ->eventID_.subRun()
               << setw(15) << " "
               << setw(15) << it->entry_ << "  (SubRun)"
               << "\n";
         }
         else if (it->getEntryType() == FileIndex::kRun) {
            os << setw(15) << it->eventID_.run()
               << setw(15) << " "
               << setw(15) << " "
               << setw(15) << it->entry_ << "  (Run)"
               << "\n";
         }
      }
      return os;
   }

   FileIndex::const_iterator
   FileIndex::findEventForUnspecifiedSubRun(EventID const &eID,
                                            bool exact) const {

      RunID const &runID = eID.runID();
      EventNumber_t event = eID.event();
      SubRunID last_subRunID;

      // Try to find the event.
      const_iterator it = findEventPosition
         (EventID::firstEvent
          (SubRunID::firstSubRun(runID)),
          false);
      const_iterator itEnd = entries_.end();
      if (it == itEnd) return it;

      // Starting with start_it, jump to the first event of each subrun
      // until we find either:
      //
      // 1. The next run.
      // 2. An event number higher than we want.
      // 3. The end of the file index.
      while ((it != itEnd) &&
             (it->eventID_.runID() == runID) &&
             (it->eventID_.event() < event)) {
         last_subRunID = it->eventID_.subRunID();
         it = findEventPosition(EventID::invalidEvent(it->eventID_.subRunID().next()), false);
      }

      if ((it != itEnd) &&
          (it->eventID_.runID() == runID) &&
          (it->eventID_.event() == event)) {
         // We started on the correct event.
         return it;
      } else if (last_subRunID.isValid()) {
         // Find the event.
         return findEventPosition(EventID(last_subRunID, event), exact);
      } else {
         // Did not find anything.
         return itEnd;
      }   
   }
}
