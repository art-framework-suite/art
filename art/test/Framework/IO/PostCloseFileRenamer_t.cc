#define BOOST_TEST_MODULE (PostCloseFileRenamer_t)
#include "cetlib/quiet_unit_test.hpp"

#include "art/Framework/IO/FileStatsCollector.h"
#include "art/Framework/IO/PostCloseFileRenamer.h"
#include "boost/filesystem.hpp"
#include "canvas/Persistency/Provenance/Timestamp.h"

extern "C" {
#include <sys/stat.h> // mkdir().
#include <unistd.h>   // chdir().
}

#include <chrono>
#include <thread>

using art::FileStatsCollector;
using art::PostCloseFileRenamer;
using namespace std::chrono_literals;
using namespace std::string_literals;

namespace {
  struct TestFixture {
    void simulateJob();

    FileStatsCollector fstats{"label", "DEVEL"};
  };

  void
  TestFixture::simulateJob()
  {
    fstats.recordFileOpen();
    art::EventID const e1{1, 0, 7};
    fstats.recordEvent(e1);
    fstats.recordSubRun(e1.subRunID());
    // Warning, this sleep is used for multiple tests below.
    std::this_thread::sleep_for(1s);
    art::EventID const e2{1, 1, 3};
    fstats.recordEvent(e2);
    fstats.recordSubRun(e2.subRunID());
    fstats.recordRun(e2.runID());
    art::SubRunID const sr{2, 3};
    fstats.recordSubRun(sr);
    fstats.recordRun(sr.runID());
    fstats.recordFileClose();
  }

} // namespace

BOOST_FIXTURE_TEST_SUITE(PostCloseFileRenamer_t, TestFixture)

BOOST_AUTO_TEST_CASE(constructor)
{
  PostCloseFileRenamer fr{fstats};
}

BOOST_AUTO_TEST_CASE(testSubstitutions)
{
  std::vector<std::string> const patterns{"f/stem_%r_%s_%R_%S.root"s,
                                          "f/stem_%l.root"s,
                                          "f/stem_%p.root"s,
                                          "f/stem_%5R_%2S.root"s};
  std::vector<std::string> const answers{"f/stem_1_0_2_3.root"s,
                                         "f/stem_label.root"s,
                                         "f/stem_DEVEL.root"s,
                                         "f/stem_00002_03.root"s};
  simulateJob();
  PostCloseFileRenamer fr{fstats};
  for (size_t i{0}, e = patterns.size(); i != e; ++i) {
    auto const ans = fr.applySubstitutions(patterns[i]);
    auto const& cmp = answers[i];
    BOOST_TEST(ans == cmp);
  }
}

BOOST_AUTO_TEST_CASE(resetFileOpen)
{
  simulateJob();
  PostCloseFileRenamer fr{fstats};
  std::string const pattern{"%to"};
  auto const before = fr.applySubstitutions(pattern);
  std::this_thread::sleep_for(1s);
  fstats.recordFileOpen();
  fstats.recordFileClose();
  auto const after = fr.applySubstitutions(pattern);
  BOOST_TEST(before != after);
}

BOOST_AUTO_TEST_CASE(resetEvents)
{
  PostCloseFileRenamer fr{fstats};
  std::string const pattern{"%02r_%02s_%R_%S"};
  auto const before = fr.applySubstitutions(pattern);
  auto const cmp_before = "-_-_-_-"s;
  BOOST_TEST(before == cmp_before);
  simulateJob();
  auto after = fr.applySubstitutions(pattern);
  auto const cmp_after = "01_00_2_3"s;
  BOOST_TEST(after == cmp_after);
  fstats.recordFileOpen();
  fstats.recordFileClose();
  after = fr.applySubstitutions(pattern);
  BOOST_TEST(after == cmp_before);
}

BOOST_AUTO_TEST_CASE(Runs_subruns)
{
  std::string const pattern{"%02r_%02s_%R_%S"};
  PostCloseFileRenamer fr{fstats};
  fstats.recordRun(art::RunID{7});
  BOOST_TEST(fr.applySubstitutions(pattern) == "07_-_7_-"s);
  fstats.recordSubRun(art::SubRunID{7, 0});
  BOOST_TEST(fr.applySubstitutions(pattern) == "07_00_7_0"s);
  fstats.recordSubRun(art::SubRunID{7, 5});
  BOOST_TEST(fr.applySubstitutions(pattern) == "07_00_7_5"s);
  fstats.recordRun(art::RunID{7});
  BOOST_TEST(fr.applySubstitutions(pattern) == "07_00_7_5"s);
  fstats.recordRun(art::RunID{9});
  BOOST_TEST(fr.applySubstitutions(pattern) == "07_00_9_-"s);
}

BOOST_AUTO_TEST_CASE(StartTimesRunsSubruns)
{
  PostCloseFileRenamer fr{fstats};
  BOOST_TEST(fr.applySubstitutions("%tr") == "-"s);
  BOOST_TEST(fr.applySubstitutions("%ts") == "-"s);
  BOOST_TEST(fr.applySubstitutions("%tR") == "-"s);
  BOOST_TEST(fr.applySubstitutions("%tS") == "-"s);
  simulateJob();
  BOOST_TEST(fr.applySubstitutions("%tr") != "-"s);
  BOOST_TEST(fr.applySubstitutions("%ts") != "-"s);
  BOOST_TEST(fr.applySubstitutions("%tR") != "-"s);
  BOOST_TEST(fr.applySubstitutions("%tS") != "-"s);
}

BOOST_AUTO_TEST_CASE(SeqNo1)
{
  std::string const pattern{"ethel-%02#-charlie"};
  PostCloseFileRenamer fr{fstats};
  BOOST_TEST(fr.applySubstitutions(pattern) == "ethel-00-charlie"s);
  fstats.recordFileOpen();
  BOOST_TEST(fr.applySubstitutions(pattern) == "ethel-00-charlie"s);
  fstats.recordFileClose();
  BOOST_TEST(fr.applySubstitutions(pattern) == "ethel-01-charlie"s);
  fstats.recordFileOpen();
  BOOST_TEST(fr.applySubstitutions(pattern) == "ethel-01-charlie"s);
  fstats.recordFileClose();
  BOOST_TEST(fr.applySubstitutions(pattern) == "ethel-02-charlie"s);
}

BOOST_AUTO_TEST_CASE(SeqNo2)
{
  std::string const pattern{"ethel-%02#-charlie-%#-bertha"};
  PostCloseFileRenamer fr{fstats};
  BOOST_TEST(fr.applySubstitutions(pattern) == "ethel-00-charlie-0-bertha"s);
  fstats.recordFileOpen();
  BOOST_TEST(fr.applySubstitutions(pattern) == "ethel-00-charlie-0-bertha"s);
  fstats.recordFileClose();
  BOOST_TEST(fr.applySubstitutions(pattern) == "ethel-01-charlie-1-bertha"s);
}

BOOST_AUTO_TEST_CASE(SimpleFileNameSubs)
{
  std::string const pattern{"silly_%ifb_%ifd_%ife_%ifn_%ifp.root"};
  PostCloseFileRenamer fr{fstats};

  // Empty.
  fstats.recordInputFile("");
  BOOST_TEST(fr.applySubstitutions(pattern) == "silly_-_-_-_-_-.root"s);

  // Simple.
  auto owd = boost::filesystem::current_path();
  auto tmpdir =
    boost::filesystem::canonical(boost::filesystem::temp_directory_path())
      .native();
  BOOST_TEST_REQUIRE(chdir(tmpdir.c_str()) == 0);

  fstats.recordInputFile("fileonlynoext");
  BOOST_TEST(fr.applySubstitutions(pattern) ==
             std::string{"silly_fileonlynoext_" + tmpdir + "__fileonlynoext_" +
                         tmpdir + "/fileonlynoext.root"});

  // Relative.
  auto uniqueDir = boost::filesystem::unique_path("%%%%-%%%%-%%%%-%%%%");
  BOOST_TEST_REQUIRE(mkdir(uniqueDir.native().c_str(), 0755) == 0);
  auto absUniqueDir = boost::filesystem::canonical(uniqueDir).native();
  fstats.recordInputFile((uniqueDir / "x.ext").native());
  BOOST_TEST(fr.applySubstitutions(pattern) ==
             std::string{"silly_x_" + absUniqueDir + "_.ext_x.ext_" +
                         absUniqueDir + "/x.ext.root"});
  BOOST_TEST_REQUIRE(chdir(owd.native().c_str()) == 0);

  // Absolute.
  fstats.recordInputFile("/usr/bin/y.ext");
  BOOST_TEST(fr.applySubstitutions(pattern) ==
             "silly_y_/usr/bin_.ext_y.ext_/usr/bin/y.ext.root"s);
}

BOOST_AUTO_TEST_CASE(SimpleRegex)
{
  std::string const pattern{"%ifs%root%dat%%"};
  PostCloseFileRenamer fr{fstats};
  fstats.recordInputFile("/tmp/c.root");
  BOOST_TEST(fr.applySubstitutions(pattern) == "/tmp/c.dat"s);
}

BOOST_AUTO_TEST_CASE(EscapingRegex)
{
  std::string const pattern{"%ifs%\\.root%.dat%%"};
  PostCloseFileRenamer fr{fstats};
  fstats.recordInputFile("/tmp/c.root");
  BOOST_TEST(fr.applySubstitutions(pattern) == "/tmp/c.dat"s);
}

BOOST_AUTO_TEST_CASE(iFlagRegex)
{
  std::string const pattern{"%ifs%ROOT%dat%i%"};
  PostCloseFileRenamer fr{fstats};
  fstats.recordInputFile("/tmp/c.root");
  BOOST_TEST(fr.applySubstitutions(pattern) == "/tmp/c.dat"s);
}

BOOST_AUTO_TEST_CASE(gFlagRegex)
{
  std::string const pattern{"%ifs%o%f%g%"};
  PostCloseFileRenamer fr{fstats};
  fstats.recordInputFile("/tmp/c.root");
  BOOST_TEST(fr.applySubstitutions(pattern) == "/tmp/c.rfft"s);
}

BOOST_AUTO_TEST_CASE(GroupingRegex)
{
  std::string const pattern{"%ifd/d_%ifs%^.*?_([\\d]+).*$%${1}_charlie%%%ife"};
  auto tmpdir = boost::filesystem::canonical("/tmp");
  PostCloseFileRenamer fr{fstats};
  fstats.recordInputFile("/tmp/c_27_ethel.root");
  BOOST_TEST(fr.applySubstitutions(pattern) ==
             (tmpdir / "d_27_charlie.root").native());
}

BOOST_AUTO_TEST_SUITE_END()
