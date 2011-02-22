/*
 * fileIndex_t.cppunit.cc
 */

#include <cppunit/extensions/HelperMacros.h>

#include "art/Persistency/Provenance/FileIndex.h"

class testFileIndex: public CppUnit::TestFixture {
   CPPUNIT_TEST_SUITE(testFileIndex);
   CPPUNIT_TEST(constructAndInsertTest);
   CPPUNIT_TEST(eventSortAndSearchTest);
   CPPUNIT_TEST(eventEntrySortAndSearchTest);
   CPPUNIT_TEST(eventsUniqueAndOrderedTest);
   CPPUNIT_TEST_SUITE_END();

public:
   void setUp() { }
   void tearDown() { }

   void constructAndInsertTest();
   void eventSortAndSearchTest();
   void eventEntrySortAndSearchTest();
   void eventsUniqueAndOrderedTest();

   bool areEntryVectorsTheSame(art::FileIndex &i1, art::FileIndex &i2);
};

///registration of the test so that the runner can find it
CPPUNIT_TEST_SUITE_REGISTRATION(testFileIndex);

void testFileIndex::constructAndInsertTest() {
   art::FileIndex fileIndex;
   CPPUNIT_ASSERT(fileIndex.empty());
   CPPUNIT_ASSERT(fileIndex.size() == 0);
   CPPUNIT_ASSERT(fileIndex.begin() == fileIndex.end());

   fileIndex.addEntry(art::EventID(100, 101, 102), 1);
   fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(200, 201)), 2);
   fileIndex.addEntry(art::EventID(300, 0, 1), 3);
   fileIndex.addEntry(art::EventID::invalidEvent(art::RunID(300)), 3);
   fileIndex.addEntry(art::EventID(100, 101, 103), 3);

   CPPUNIT_ASSERT(!fileIndex.empty());
   CPPUNIT_ASSERT(fileIndex.size() == 5);

   art::FileIndex::const_iterator iter = fileIndex.begin();
   CPPUNIT_ASSERT(iter->eventID_ == art::EventID(100, 101, 102));
   CPPUNIT_ASSERT(iter->entry_ == 1);
   CPPUNIT_ASSERT(iter->getEntryType() == art::FileIndex::kEvent);

   ++iter;
   CPPUNIT_ASSERT(iter->getEntryType() == art::FileIndex::kSubRun);

   ++iter;
   CPPUNIT_ASSERT(iter->getEntryType() == art::FileIndex::kEvent);

   ++iter;
   CPPUNIT_ASSERT(iter->eventID_ == art::EventID::invalidEvent(art::RunID(300)));
   CPPUNIT_ASSERT(iter->entry_ == 3);
   CPPUNIT_ASSERT(iter->getEntryType() == art::FileIndex::kRun);

   ++iter;
   ++iter;
   CPPUNIT_ASSERT(iter == fileIndex.end());

   CPPUNIT_ASSERT(fileIndex.allEventsInEntryOrder() == true);
   CPPUNIT_ASSERT(fileIndex.allEventsInEntryOrder() == true);
}

void testFileIndex::eventSortAndSearchTest()
{
 // Note this contains some illegal duplicates
 // For the moment there is nothing to prevent
 // these from occurring so we handle this using
 // a stable_sort for now ... They will not bother
 // the FileIndex.

 art::FileIndex fileIndex;
 fileIndex.addEntry(art::EventID(3, 3, 2), 5);
 fileIndex.addEntry(art::EventID(3, 3, 1), 4);
 fileIndex.addEntry(art::EventID(3, 3, 3), 3);
 fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 3)), 6);
 fileIndex.addEntry(art::EventID::invalidEvent(art::RunID(3)), 7);
 fileIndex.addEntry(art::EventID::invalidEvent(art::RunID(1)), 8);
 fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 1)), 9);
 fileIndex.addEntry(art::EventID::invalidEvent(art::RunID(3)), 1);
 fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 3)), 1);
 fileIndex.addEntry(art::EventID(3, 3, 1), 1);
 fileIndex.addEntry(art::EventID(1, 2, 2), 2);
 fileIndex.addEntry(art::EventID(1, 2, 4), 1);
 fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 2);
 fileIndex.addEntry(art::EventID(1, 2, 1), 2);

 fileIndex.sortBy_Run_SubRun_Event();

 art::FileIndex shouldBe;
 shouldBe.addEntry(art::EventID::invalidEvent(art::RunID(1)), 8);
 shouldBe.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 2);
 shouldBe.addEntry(art::EventID(1, 2, 1), 2);
 shouldBe.addEntry(art::EventID(1, 2, 2), 2);
 shouldBe.addEntry(art::EventID(1, 2, 4), 1);
 shouldBe.addEntry(art::EventID::invalidEvent(art::RunID(3)), 7);
 shouldBe.addEntry(art::EventID::invalidEvent(art::RunID(3)), 1);
 shouldBe.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 1)), 9);
 shouldBe.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 3)), 6);
 shouldBe.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 3)), 1);
 shouldBe.addEntry(art::EventID(3, 3, 1), 4);
 shouldBe.addEntry(art::EventID(3, 3, 1), 1);
 shouldBe.addEntry(art::EventID(3, 3, 2), 5);
 shouldBe.addEntry(art::EventID(3, 3, 3), 3);

 CPPUNIT_ASSERT(areEntryVectorsTheSame(fileIndex, shouldBe));

 CPPUNIT_ASSERT(fileIndex.allEventsInEntryOrder() == false);
 CPPUNIT_ASSERT(fileIndex.allEventsInEntryOrder() == false);

 art::FileIndex::const_iterator iter = fileIndex.findPosition(art::EventID(3, 3, 2));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 12);
 CPPUNIT_ASSERT(iter->eventID_ == art::EventID(3, 3, 2));
 CPPUNIT_ASSERT(iter->entry_ == 5);

 iter = fileIndex.findPosition(art::EventID(3, 3, 7));
 CPPUNIT_ASSERT(iter == fileIndex.end());

 iter = fileIndex.findPosition(art::EventID(1, 2, 3));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 4);

 iter = fileIndex.findPosition(art::EventID::invalidEvent(art::SubRunID(3, 3)));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

 iter = fileIndex.findPosition(art::EventID::invalidEvent(art::SubRunID(1, 1)));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 1);

 iter = fileIndex.findPosition(art::EventID::invalidEvent(art::RunID(1)));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 0);

 iter = fileIndex.findPosition(art::EventID::invalidEvent(art::RunID(2)));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

 iter = fileIndex.findPosition(art::EventID(2, 0, 1));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

 iter = fileIndex.findPosition(art::EventID(1, 2, 3));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 4);

 iter = fileIndex.findPosition(art::EventID(3, 0, 3));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 13);

 iter = fileIndex.findEventPosition(art::EventID(3, 3, 2), true);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 12);
 CPPUNIT_ASSERT(fileIndex.containsEvent(art::EventID(3, 3, 2), true));

 iter = fileIndex.findEventPosition(art::EventID(1, 2, 3), true);
 CPPUNIT_ASSERT(iter == fileIndex.end());
 CPPUNIT_ASSERT(!fileIndex.containsEvent(art::EventID(1, 2, 3), true));

 iter = fileIndex.findEventPosition(art::EventID(1, 2, 3), false);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 4);
 CPPUNIT_ASSERT(fileIndex.containsEvent(art::EventID(1, 2, 3), false));

 iter = fileIndex.findEventPosition(art::EventID(3, 0, 1), true);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 10);

 iter = fileIndex.findSubRunPosition(art::SubRunID(3, 1), true);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 7);

 iter = fileIndex.findSubRunPosition(art::SubRunID(3, 2), true);
 CPPUNIT_ASSERT(iter == fileIndex.end());

 iter = fileIndex.findSubRunPosition(art::SubRunID(3, 1), false);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 7);

 iter = fileIndex.findSubRunPosition(art::SubRunID(3, 2), false);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

 CPPUNIT_ASSERT(fileIndex.containsSubRun(art::SubRunID(3, 3), true));
 CPPUNIT_ASSERT(!fileIndex.containsSubRun(art::SubRunID(2, 3), true));

 iter = fileIndex.findRunPosition(art::RunID(3), true);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

 iter = fileIndex.findRunPosition(art::RunID(2), true);
 CPPUNIT_ASSERT(iter == fileIndex.end());

 iter = fileIndex.findRunPosition(art::RunID(2), false);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

 CPPUNIT_ASSERT(fileIndex.containsRun(art::RunID(3), true));
 CPPUNIT_ASSERT(!fileIndex.containsRun(art::RunID(2), true));

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID(1, 2));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 1);

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID::invalidSubRun(art::RunID(3)));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID(3, 4));
 CPPUNIT_ASSERT(iter == fileIndex.end());

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID(3, 2));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID(2, 1));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

}
void testFileIndex::eventEntrySortAndSearchTest()
{

 // Note this contains some illegal duplicates
 // For the moment there is nothing to prevent
 // these from occurring so we handle this using
 // a stable_sort for now ... They will not bother
 // the FileIndex.

 art::FileIndex fileIndex;
 fileIndex.addEntry(art::EventID(3, 3, 2), 5);
 fileIndex.addEntry(art::EventID(3, 3, 1), 4);
 fileIndex.addEntry(art::EventID(3, 3, 3), 3);
 fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 3)), 6);
 fileIndex.addEntry(art::EventID::invalidEvent(art::RunID(3)), 7);
 fileIndex.addEntry(art::EventID::invalidEvent(art::RunID(1)), 8);
 fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 1)), 9);
 fileIndex.addEntry(art::EventID::invalidEvent(art::RunID(3)), 1);
 fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 3)), 1);
 fileIndex.addEntry(art::EventID(3, 3, 1), 1);
 fileIndex.addEntry(art::EventID(1, 2, 2), 2);
 fileIndex.addEntry(art::EventID(1, 2, 4), 1);
 fileIndex.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 2);
 fileIndex.addEntry(art::EventID(1, 2, 1), 2);

 fileIndex.sortBy_Run_SubRun_EventEntry();

 art::FileIndex shouldBe;
 shouldBe.addEntry(art::EventID::invalidEvent(art::RunID(1)), 8);
 shouldBe.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 2);
 shouldBe.addEntry(art::EventID(1, 2, 4), 1);
 shouldBe.addEntry(art::EventID(1, 2, 2), 2);
 shouldBe.addEntry(art::EventID(1, 2, 1), 2);
 shouldBe.addEntry(art::EventID::invalidEvent(art::RunID(3)), 7);
 shouldBe.addEntry(art::EventID::invalidEvent(art::RunID(3)), 1);
 shouldBe.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 1)), 9);
 shouldBe.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 3)), 6);
 shouldBe.addEntry(art::EventID::invalidEvent(art::SubRunID(3, 3)), 1);
 shouldBe.addEntry(art::EventID(3, 3, 1), 1);
 shouldBe.addEntry(art::EventID(3, 3, 3), 3);
 shouldBe.addEntry(art::EventID(3, 3, 1), 4);
 shouldBe.addEntry(art::EventID(3, 3, 2), 5);

 CPPUNIT_ASSERT(areEntryVectorsTheSame(fileIndex, shouldBe));

 art::FileIndex::const_iterator iter = fileIndex.findSubRunPosition(art::SubRunID(3, 1), true);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 7);

 iter = fileIndex.findSubRunPosition(art::SubRunID(3, 2), true);
 CPPUNIT_ASSERT(iter == fileIndex.end());

 iter = fileIndex.findSubRunPosition(art::SubRunID(3, 1), false);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 7);

 iter = fileIndex.findSubRunPosition(art::SubRunID(3, 2), false);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

 iter = fileIndex.findRunPosition(art::RunID(3), true);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

 iter = fileIndex.findRunPosition(art::RunID(2), true);
 CPPUNIT_ASSERT(iter == fileIndex.end());

 iter = fileIndex.findRunPosition(art::RunID(2), false);
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID(1, 2));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 1);

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID::invalidSubRun(art::RunID(3)));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID(3, 4));
 CPPUNIT_ASSERT(iter == fileIndex.end());

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID(3, 2));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 8);

 iter = fileIndex.findSubRunOrRunPosition(art::SubRunID(2, 1));
 CPPUNIT_ASSERT((iter - fileIndex.begin()) == 5);
}

void testFileIndex::eventsUniqueAndOrderedTest() {

 // Test the different cases

 // Nothing in the FileIndex
 art::FileIndex fileIndex;
 CPPUNIT_ASSERT(fileIndex.eventsUniqueAndOrdered());

 // No events
 art::FileIndex fileIndex1;
 fileIndex1.addEntry(art::EventID::invalidEvent(art::RunID(1)), 1);
 fileIndex1.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 1)), 1);
 CPPUNIT_ASSERT(fileIndex1.eventsUniqueAndOrdered());

 // One event and nothing after it
 art::FileIndex fileIndex2;
 fileIndex2.addEntry(art::EventID::invalidEvent(art::RunID(1)), 1);
 fileIndex2.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 1);
 fileIndex2.addEntry(art::EventID(1, 2, 1), 1);
 CPPUNIT_ASSERT(fileIndex2.eventsUniqueAndOrdered());

 // One event with a run after it
 art::FileIndex fileIndex3;
 fileIndex3.addEntry(art::EventID::invalidEvent(art::RunID(1)), 1);
 fileIndex3.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 1);
 fileIndex3.addEntry(art::EventID(1, 2, 1), 1);
 fileIndex3.addEntry(art::EventID::invalidEvent(art::RunID(2)), 2);
 CPPUNIT_ASSERT(fileIndex3.eventsUniqueAndOrdered());

 // Two events
 art::FileIndex fileIndex4;
 fileIndex4.addEntry(art::EventID::invalidEvent(art::RunID(1)), 1);
 fileIndex4.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 1)), 1);
 fileIndex4.addEntry(art::EventID(1, 1, 1), 1);
 fileIndex4.addEntry(art::EventID::invalidEvent(art::RunID(2)), 2);
 fileIndex4.addEntry(art::EventID::invalidEvent(art::SubRunID(2, 1)), 2);
 fileIndex4.addEntry(art::EventID(2, 1, 1), 2);
 CPPUNIT_ASSERT(fileIndex4.eventsUniqueAndOrdered());

 // Two events, same run and event number
 art::FileIndex fileIndex5;
 fileIndex5.addEntry(art::EventID::invalidEvent(art::RunID(1)), 1);
 fileIndex5.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 1)), 1);
 fileIndex5.addEntry(art::EventID(1, 1, 1), 1);
 fileIndex5.addEntry(art::EventID::invalidEvent(art::RunID(1)), 2);
 fileIndex5.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 2);
 fileIndex5.addEntry(art::EventID(1, 2, 1), 2);
 CPPUNIT_ASSERT(!fileIndex5.eventsUniqueAndOrdered());

 // Not ordered by run
 art::FileIndex fileIndex6;
 fileIndex6.addEntry(art::EventID::invalidEvent(art::RunID(1)), 1);
 fileIndex6.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 1);
 fileIndex6.addEntry(art::EventID(1, 2, 1), 1);
 fileIndex6.addEntry(art::EventID::invalidEvent(art::RunID(2)), 2);
 fileIndex6.addEntry(art::EventID::invalidEvent(art::SubRunID(2, 1)), 2);
 fileIndex6.addEntry(art::EventID(2, 1, 1), 2);
 fileIndex6.addEntry(art::EventID::invalidEvent(art::RunID(1)), 3);
 fileIndex6.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 3)), 3);
 fileIndex6.addEntry(art::EventID(1, 3, 1), 3);
 CPPUNIT_ASSERT(!fileIndex6.eventsUniqueAndOrdered());

 // Not ordered by event
 art::FileIndex fileIndex7;
 fileIndex7.addEntry(art::EventID::invalidEvent(art::RunID(1)), 1);
 fileIndex7.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 2)), 1);
 fileIndex7.addEntry(art::EventID(1, 2, 1), 1);
 fileIndex7.addEntry(art::EventID::invalidEvent(art::RunID(2)), 2);
 fileIndex7.addEntry(art::EventID::invalidEvent(art::SubRunID(2, 1)), 2);
 fileIndex7.addEntry(art::EventID(2, 1, 2), 2);
 fileIndex7.addEntry(art::EventID::invalidEvent(art::SubRunID(2, 3)), 3);
 fileIndex7.addEntry(art::EventID(2, 3, 1), 3);
 CPPUNIT_ASSERT(!fileIndex7.eventsUniqueAndOrdered());

 // OK, ordered by event and unique
 art::FileIndex fileIndex8;
 fileIndex8.addEntry(art::EventID::invalidEvent(art::RunID(1)), 1);
 fileIndex8.addEntry(art::EventID::invalidEvent(art::SubRunID(1, 1)), 1);
 fileIndex8.addEntry(art::EventID(1, 1, 1), 1);
 fileIndex8.addEntry(art::EventID(1, 1, 2), 1);
 fileIndex8.addEntry(art::EventID(1, 1, 3), 1);
 fileIndex8.addEntry(art::EventID(1, 1, 4), 1);
 fileIndex8.addEntry(art::EventID::invalidEvent(art::RunID(2)), 2);
 fileIndex8.addEntry(art::EventID::invalidEvent(art::SubRunID(2, 1)), 2);
 fileIndex8.addEntry(art::EventID(2, 1, 1), 2);
 fileIndex8.addEntry(art::EventID::invalidEvent(art::SubRunID(2, 3)), 3);
 fileIndex8.addEntry(art::EventID(2, 3, 2), 3);
 fileIndex8.addEntry(art::EventID(2, 3, 3), 3);
 fileIndex8.addEntry(art::EventID(2, 3, 4), 3);
 fileIndex8.addEntry(art::EventID(2, 3, 5), 3);
 fileIndex8.addEntry(art::EventID(2, 3, 6), 3);
 CPPUNIT_ASSERT(fileIndex8.eventsUniqueAndOrdered());
}

bool testFileIndex::areEntryVectorsTheSame(art::FileIndex &i1, art::FileIndex &i2) {
   if (i1.size() != i2.size()) return false;
   for (art::FileIndex::const_iterator
           iter1 = i1.begin(),
           iter2 = i2.begin();
        iter1 != i1.end();
        ++iter1, ++iter2) {
      if (*iter1 != *iter2) return false;
   }
   return true;
}
