#define BOOST_TEST_MODULE ( PostCloseFileRenamer_t )
#include "boost/test/auto_unit_test.hpp"

#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "boost/filesystem.hpp"

extern "C" {
#include <unistd.h> // chdir().
#include <sys/stat.h> // mkdir().
}

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

BOOST_AUTO_TEST_CASE(SeqNo)
{
  PostCloseFileRenamer fr("ethel-%02#-charlie", "label", "DEVEL");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("ethel-00-charlie"));
  fr.recordFileOpen();
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("ethel-00-charlie"));
  fr.recordFileClose();
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("ethel-01-charlie"));
  fr.recordFileOpen();
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("ethel-01-charlie"));
  fr.recordFileClose();
  BOOST_CHECK_EQUAL(fr.applySubstitutions(), std::string("ethel-02-charlie"));
}

BOOST_AUTO_TEST_CASE(SimpleFileNameSubs)
{
  PostCloseFileRenamer fr("silly_%ifb_%ifd_%ife_%ifn_%ifp.root",
                          "label",
                          "DEVEL");
  // Empty.
  fr.recordInputFile("");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    std::string("silly_-_-_-_-_-.root"));

  // Simple.
  auto owd = boost::filesystem::current_path();
  auto tmpdir = boost::filesystem::canonical(boost::filesystem::temp_directory_path()).native();
  BOOST_REQUIRE_EQUAL(chdir(tmpdir.c_str()), 0);

  fr.recordInputFile("fileonlynoext");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    std::string("silly_fileonlynoext_" + tmpdir +
                                "__fileonlynoext_" + tmpdir +
                                "/fileonlynoext.root"));

  // Relative.
  auto uniqueDir = boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%");
  BOOST_REQUIRE_EQUAL(mkdir(uniqueDir.native().c_str(), 0755), 0);
  auto absUniqueDir = boost::filesystem::canonical(uniqueDir).native();
  fr.recordInputFile((uniqueDir / "x.ext").native());
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    std::string("silly_x_" + absUniqueDir +
                                "_.ext_x.ext_" + absUniqueDir +
                                "/x.ext.root"));
  BOOST_REQUIRE_EQUAL(chdir(owd.native().c_str()), 0);

  // Absolute.
  fr.recordInputFile("/usr/bin/y.ext");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    std::string("silly_y_/usr/bin_.ext_y.ext_/usr/bin/y.ext.root"));
}

BOOST_AUTO_TEST_CASE(SimpleRegex)
{
  PostCloseFileRenamer fr("%ifs%root%dat%%", "label", "DEVEL");
  fr.recordInputFile("/tmp/c.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    std::string("/tmp/c.dat"));
}

BOOST_AUTO_TEST_CASE(EscapingRegex)
{
  PostCloseFileRenamer fr("%ifs%\\.root%.dat%%", "label", "DEVEL");
  fr.recordInputFile("/tmp/c.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    std::string("/tmp/c.dat"));
}

BOOST_AUTO_TEST_CASE(iFlagRegex)
{
  PostCloseFileRenamer fr("%ifs%ROOT%dat%i%", "label", "DEVEL");
  fr.recordInputFile("/tmp/c.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    std::string("/tmp/c.dat"));
}

BOOST_AUTO_TEST_CASE(gFlagRegex)
{
  PostCloseFileRenamer fr("%ifs%o%f%g%", "label", "DEVEL");
  fr.recordInputFile("/tmp/c.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    std::string("/tmp/c.rfft"));
}

BOOST_AUTO_TEST_CASE(GroupingRegex)
{
  auto tmpdir = boost::filesystem::canonical("/tmp");
  PostCloseFileRenamer fr("%ifd/d_%ifs%^.*?_([\\d]+).*$%${1}_charlie%%%ife", "label", "DEVEL");
  fr.recordInputFile("/tmp/c_27_ethel.root");
  BOOST_CHECK_EQUAL(fr.applySubstitutions(),
                    (tmpdir / "d_27_charlie.root").native());
}

BOOST_AUTO_TEST_SUITE_END()
