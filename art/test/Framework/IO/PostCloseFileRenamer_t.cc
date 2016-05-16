#define BOOST_TEST_MODULE ( PostCloseFileRenamer_t )
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "boost/filesystem.hpp"

extern "C" {
#include <unistd.h> // chdir().
#include <sys/stat.h> // mkdir().
}

using art::FileStatsCollector;
using art::PostCloseFileRenamer;

namespace {
  struct TestFixture {
    TestFixture();
    void simulateJob();

    FileStatsCollector fstats;
  };

  TestFixture::TestFixture()
    :
    fstats("label", "DEVEL")
  {
  }

  void
  TestFixture::
  simulateJob()
  {
    fstats.recordFileOpen();
    std::vector<art::EventID> eIDs { {1, 0, 7}, { 1, 1, 3 }, { 2, 3, 1} };
    for (auto const & eID : eIDs) {
      fstats.recordEvent(eID);
    }
    fstats.recordFileClose();
  }

}


BOOST_FIXTURE_TEST_SUITE(PostCloseFileRenamer_t, TestFixture)

BOOST_AUTO_TEST_CASE(constructor)
{
  PostCloseFileRenamer fr(fstats);
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
  simulateJob();
  PostCloseFileRenamer fr(fstats);
  for (size_t i { 0 }, e = patterns.size(); i != e; ++i) {
    auto const ans = fr.applySubstitutions(patterns[i]);
    auto const & cmp = answers[i];
    BOOST_CHECK_EQUAL(ans, cmp);
  }
}

BOOST_AUTO_TEST_CASE(resetFileOpen)
{
  simulateJob();
  PostCloseFileRenamer fr(fstats);
  std::string const pattern("%to");
  auto const before = fr.applySubstitutions(pattern);
  sleep(1);
  fstats.recordFileOpen();
  fstats.recordFileClose();
  auto const after = fr.applySubstitutions(pattern);
  BOOST_CHECK_NE(before, after);
}

BOOST_AUTO_TEST_CASE(resetEvents)
{
  PostCloseFileRenamer fr(fstats);
  std::string const pattern("%02r_%02s_%R_%S");
  auto const before = fr.applySubstitutions(pattern);
  auto const cmp_before = std::string("-_-_-_-");
  BOOST_CHECK_EQUAL(before, cmp_before);
  simulateJob();
  auto after = fr.applySubstitutions(pattern);
  auto const cmp_after = std::string("01_00_2_3");
  BOOST_CHECK_EQUAL(after, cmp_after);
  fstats.recordFileOpen();
  fstats.recordFileClose();
  after = fr.applySubstitutions(pattern);
  BOOST_CHECK_EQUAL(after, cmp_before);
}

BOOST_AUTO_TEST_CASE(Runs_subruns)
{
  std::string const pattern("%02r_%02s_%R_%S");
  PostCloseFileRenamer fr(fstats);
  fstats.recordRun(art::RunID{ 7 });
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("07_-_7_-"));
  fstats.recordSubRun(art::SubRunID{ 7, 0 });
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("07_00_7_0"));
  fstats.recordSubRun(art::SubRunID{ 7, 5 });
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("07_00_7_5"));
  fstats.recordRun(art::RunID{ 9 });
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("07_00_9_-"));
}

BOOST_AUTO_TEST_CASE(SeqNo)
{
  std::string const pattern("ethel-%02#-charlie");
  PostCloseFileRenamer fr(fstats);
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("ethel-00-charlie"));
  fstats.recordFileOpen();
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("ethel-00-charlie"));
  fstats.recordFileClose();
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("ethel-01-charlie"));
  fstats.recordFileOpen();
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("ethel-01-charlie"));
  fstats.recordFileClose();
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern), std::string("ethel-02-charlie"));
}

BOOST_AUTO_TEST_CASE(SimpleFileNameSubs)
{
  std::string const pattern("silly_%ifb_%ifd_%ife_%ifn_%ifp.root");
  PostCloseFileRenamer fr(fstats);

  // Empty.
  fstats.recordInputFile("");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    std::string("silly_-_-_-_-_-.root"));

  // Simple.
  auto owd = boost::filesystem::current_path();
  auto tmpdir = boost::filesystem::canonical(boost::filesystem::temp_directory_path()).native();
  BOOST_REQUIRE_EQUAL(chdir(tmpdir.c_str()), 0);

  fstats.recordInputFile("fileonlynoext");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    std::string("silly_fileonlynoext_" + tmpdir +
                                "__fileonlynoext_" + tmpdir +
                                "/fileonlynoext.root"));

  // Relative.
  auto uniqueDir = boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%");
  BOOST_REQUIRE_EQUAL(mkdir(uniqueDir.native().c_str(), 0755), 0);
  auto absUniqueDir = boost::filesystem::canonical(uniqueDir).native();
  fstats.recordInputFile((uniqueDir / "x.ext").native());
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    std::string("silly_x_" + absUniqueDir +
                                "_.ext_x.ext_" + absUniqueDir +
                                "/x.ext.root"));
  BOOST_REQUIRE_EQUAL(chdir(owd.native().c_str()), 0);

  // Absolute.
  fstats.recordInputFile("/usr/bin/y.ext");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    std::string("silly_y_/usr/bin_.ext_y.ext_/usr/bin/y.ext.root"));
}

BOOST_AUTO_TEST_CASE(SimpleRegex)
{
  std::string const pattern("%ifs%root%dat%%");
  PostCloseFileRenamer fr(fstats);
  fstats.recordInputFile("/tmp/c.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    std::string("/tmp/c.dat"));
}

BOOST_AUTO_TEST_CASE(EscapingRegex)
{
  std::string const pattern("%ifs%\\.root%.dat%%");
  PostCloseFileRenamer fr(fstats);
  fstats.recordInputFile("/tmp/c.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    std::string("/tmp/c.dat"));
}

BOOST_AUTO_TEST_CASE(iFlagRegex)
{
  std::string const pattern("%ifs%ROOT%dat%i%");
  PostCloseFileRenamer fr(fstats);
  fstats.recordInputFile("/tmp/c.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    std::string("/tmp/c.dat"));
}

BOOST_AUTO_TEST_CASE(gFlagRegex)
{
  std::string const pattern("%ifs%o%f%g%");
  PostCloseFileRenamer fr(fstats);
  fstats.recordInputFile("/tmp/c.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    std::string("/tmp/c.rfft"));
}

BOOST_AUTO_TEST_CASE(GroupingRegex)
{
  std::string const pattern("%ifd/d_%ifs%^.*?_([\\d]+).*$%${1}_charlie%%%ife");
  auto tmpdir = boost::filesystem::canonical("/tmp");
  PostCloseFileRenamer fr(fstats);
  fstats.recordInputFile("/tmp/c_27_ethel.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(pattern),
                    (tmpdir / "d_27_charlie.root").native());
}

BOOST_AUTO_TEST_SUITE_END()
