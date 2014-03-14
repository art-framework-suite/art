#define BOOST_TEST_MODULE ( PostCloseFileRenamer_t )
#include "boost/test/auto_unit_test.hpp"

#include "art/Framework/IO/PostCloseFileRenamer.h"

using art::PostCloseFileRenamer;

namespace {
  void simulateJob(PostCloseFileRenamer & fr)
  {
    fr.recordFileOpen();
    std::vector<art::EventID> eIDs { {1, 0, 7}, { 1, 1, 3 }, { 2, 3, 1} };
    for (auto const & eID : eIDs) {
      fr.recordEvent(eID);
    }
    fr.recordFileClose();
  }
}

BOOST_AUTO_TEST_SUITE(PostCloseFileRenamer_t)

BOOST_AUTO_TEST_CASE(constructor)
{
  PostCloseFileRenamer("pattern", "label", "DEVEL");
}

BOOST_AUTO_TEST_CASE(parentPath)
{
  BOOST_CHECK_EQUAL(PostCloseFileRenamer("bar", "label", "DEVEL").parentPath(), ".");
  BOOST_CHECK_EQUAL(PostCloseFileRenamer("/bar", "label", "DEVEL").parentPath(), "/");
  BOOST_CHECK_EQUAL(PostCloseFileRenamer("foo/bar", "label", "DEVEL").parentPath(), "foo");
}

BOOST_AUTO_TEST_CASE(testSubstitutions)
{
  std::vector<std::string> patterns {
    {"f/stem_%r_%s_%R_%S.root"},
    {"f/stem_%l.root"},
    {"f/stem_%p.root"},
    {"f/stem_%5R_%2S.root"}
  };
  std::vector<std::string>  answers {
    {"f/stem_1_0_2_3.root"},
    {"f/stem_label.root"},
    {"f/stem_DEVEL.root"},
    {"f/stem_00002_03.root"}
  };
  for (size_t i { 0 }, e = patterns.size(); i != e; ++i) {
    PostCloseFileRenamer fr(patterns[i], "label", "DEVEL");
    simulateJob(fr);
    auto const ans = fr.applySubstitutions();
    auto const & cmp = answers[i];
    BOOST_CHECK_EQUAL(ans, cmp);
  }
}

BOOST_AUTO_TEST_CASE(resetFileOpen)
{
  PostCloseFileRenamer fr("%to", "label", "DEVEL");
  simulateJob(fr);
  auto const before = fr.applySubstitutions();
  sleep(1);
  fr.recordFileOpen();
  fr.recordFileClose();
  auto const after = fr.applySubstitutions();
  BOOST_CHECK_NE(before, after);
}

BOOST_AUTO_TEST_CASE(resetEvents)
{
  PostCloseFileRenamer fr("%02r_%02s_%R_%S", "label", "DEVEL");
  auto const before = fr.applySubstitutions();
  auto const cmp_before = std::string("-_-_-_-");
  BOOST_CHECK_EQUAL(before, cmp_before);
  simulateJob(fr);
  auto after = fr.applySubstitutions();
  auto const cmp_after = std::string("01_00_2_3");
  BOOST_CHECK_EQUAL(after, cmp_after);
  fr.recordFileOpen();
  fr.recordFileClose();
  after = fr.applySubstitutions();
  BOOST_CHECK_EQUAL(after, cmp_before);
}

BOOST_AUTO_TEST_CASE(Runs_subruns)
{
  PostCloseFileRenamer fr("%02r_%02s_%R_%S", "label", "DEVEL");
  fr.recordRun(art::RunID{ 7 });
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("07_-_7_-"));
  fr.recordSubRun(art::SubRunID{ 7, 0 });
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("07_00_7_0"));
  fr.recordSubRun(art::SubRunID{ 7, 5 });
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("07_00_7_5"));
  fr.recordRun(art::RunID{ 9 });
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("07_00_9_-"));
}

BOOST_AUTO_TEST_SUITE_END()
