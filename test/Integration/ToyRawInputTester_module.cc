#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "fhiclcpp/ParameterSet.h"


#include <sstream>
#include <string>
#include <vector>

namespace arttest {
  class ToyRawInputTester;
}

typedef std::vector<std::string> strings;
using std::string;
using std::ostringstream;
typedef std::vector<std::vector<int> > vv_t;

using namespace art;

class arttest::ToyRawInputTester :
  public art::EDAnalyzer {
public:

  ToyRawInputTester(fhicl::ParameterSet const & p) :
    numEventsSeen_(0),
    numEventsExpected_(0),
    numSubRunsSeen_(0),
    numSubRunsExpected_(0),
    numRunsSeen_(0),
    numRunsExpected_(0),
    numFilesSeen_(0),
    numFilesExpected_(0),
    fileNames_(p.get<strings>("fileNames")),
    pset_(p),
    messages_() {
    numFilesExpected_ = fileNames_.size();
    ostringstream expected;
    for (size_t i = 0; i != numFilesExpected_; ++i) {
      expected << "open " << fileNames_[i] << '\n';
      vv_t tokens(p.get<vv_t>(fileNames_[i]));
      RunNumber_t currentRun = -1;
      SubRunNumber_t currentSubRun = -1;
      for (vv_t::const_iterator
           it = tokens.begin(),
           itend = tokens.end();
           it != itend; ++it) {
        if ((*it)[0] != -1) {
          ++numRunsExpected_;
          currentRun = (*it)[0];
          expected << "begin " << RunID(currentRun) << '\n';
        }
        if ((*it)[1] != -1) {
          ++numSubRunsExpected_;
          currentSubRun = (*it)[1];
          expected << "begin "
                   << SubRunID(currentRun, currentSubRun) << '\n';
        }
        if ((*it)[2] != -1) {
          ++numEventsExpected_;
          expected << "event "
                   << EventID(currentRun, currentSubRun, (*it)[2])
                   << '\n';
        }
      }
    }
    expectedMessage_ = expected.str();
  }

  void respondToOpenInputFile(art::FileBlock const & fb) {
    ++numFilesSeen_;
    messages_ << "open " <<  fb.fileName() << '\n';
  }

  void beginRun(art::Run const & r) {
    ++numRunsSeen_;
    messages_ << "begin " << r.id() << '\n';
  }

  void beginSubRun(art::SubRun const & sr) {
    ++numSubRunsSeen_;
    messages_ << "begin " << sr.id() << '\n';
  }

  void analyze(art::Event const & e) {
    ++numEventsSeen_;
    messages_ << "event " << e.id() << '\n';
  }

  void endJob() {
    std::cerr << "------------------------------------------------------------------------\n"
              << expectedMessage_
              << "------------------------------------------------------------------------\n"
              << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n"
              << messages_.str()
              << "++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n";
    assert(numFilesSeen_ == numFilesExpected_);
    assert(numRunsSeen_ == numRunsExpected_);
    assert(numSubRunsSeen_ == numSubRunsExpected_);
    assert(numEventsSeen_ == numEventsExpected_);
    assert(expectedMessage_ == messages_.str());
  }

private:
  size_t numEventsSeen_;
  size_t numEventsExpected_;
  size_t numSubRunsSeen_;
  size_t numSubRunsExpected_;
  size_t numRunsSeen_;
  size_t numRunsExpected_;
  size_t numFilesSeen_;
  size_t numFilesExpected_;
  strings fileNames_;
  fhicl::ParameterSet pset_;
  ostringstream messages_;
  string expectedMessage_;
};

DEFINE_ART_MODULE(arttest::ToyRawInputTester);
