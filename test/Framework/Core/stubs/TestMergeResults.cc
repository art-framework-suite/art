//
// Reads some simple test objects in the event, run, and subRun
// principals.  Then checks to see if the values in these
// objects match what we expect.  Intended to be used to
// test the values in a file that has merged run and subRun
// products.
//

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Core/FileBlock.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "test/TestObjects/Thing.h"
#include "test/TestObjects/ThingWithIsEqual.h"
#include "test/TestObjects/ThingWithMerge.h"
#include <iostream>
#include <string>
#include <vector>

namespace art {
  class EventSetup;
}

using namespace art;

namespace arttest {

  class TestMergeResults : public art::EDAnalyzer {
  public:

    explicit TestMergeResults(fhicl::ParameterSet const &);
    virtual ~TestMergeResults();

    virtual void analyze(art::Event const & e, art::EventSetup const & c);
    virtual void beginRun(Run const &, EventSetup const &);
    virtual void endRun(Run const &, EventSetup const &);
    virtual void beginSubRun(SubRun const &, EventSetup const &);
    virtual void endSubRun(SubRun const &, EventSetup const &);
    virtual void respondToOpenInputFile(FileBlock const & fb);
    virtual void respondToCloseInputFile(FileBlock const & fb);
    virtual void respondToOpenOutputFiles(FileBlock const & fb);
    virtual void respondToCloseOutputFiles(FileBlock const & fb);
    void endJob();

  private:

    void checkExpectedSubRunProducts(unsigned int index,
                                     std::vector<int> const & expectedValues,
                                     InputTag const & tag,
                                     const char * functionName,
                                     SubRun const & subRun);

    void checkExpectedRunProducts(unsigned int index,
                                  std::vector<int> const & expectedValues,
                                  InputTag const & tag,
                                  const char * functionName,
                                  Run const & run);

    void abortWithMessage(const char * whichFunction, const char * type, art::InputTag const & tag,
                          int expectedValue, int actualValue) const;

    std::vector<int> default_;
    std::vector<std::string> defaultvstring_;

    std::vector<int> expectedBeginRunProd_;
    std::vector<int> expectedEndRunProd_;
    std::vector<int> expectedBeginSubRunProd_;
    std::vector<int> expectedEndSubRunProd_;

    std::vector<int> expectedBeginRunNew_;
    std::vector<int> expectedEndRunNew_;
    std::vector<int> expectedBeginLumiNew_;
    std::vector<int> expectedEndLumiNew_;

    std::vector<std::string> expectedParents_;

    std::vector<int> expectedDroppedEvent_;

    int nRespondToOpenInputFile_;
    int nRespondToCloseInputFile_;
    int nRespondToOpenOutputFiles_;
    int nRespondToCloseOutputFiles_;
    int expectedRespondToOpenInputFile_;
    int expectedRespondToCloseInputFile_;
    int expectedRespondToOpenOutputFiles_;
    int expectedRespondToCloseOutputFiles_;

    std::vector<std::string> expectedInputFileNames_;

    bool verbose_;

    unsigned int index0_;
    unsigned int index1_;
    unsigned int index2_;
    unsigned int index3_;
    unsigned int index4_;
    unsigned int index5_;
    unsigned int index6_;
    unsigned int index7_;
    unsigned int parentIndex_;

    art::Handle<arttest::Thing> h_thing;
    art::Handle<arttest::ThingWithMerge> h_thingWithMerge;
    art::Handle<arttest::ThingWithIsEqual> h_thingWithIsEqual;
  };

  // -----------------------------------------------------------------

  TestMergeResults::TestMergeResults(fhicl::ParameterSet const & ps):
    default_(),
    defaultvstring_(),
    expectedBeginRunProd_(ps.get<std::vector<int> >("expectedBeginRunProd", default_)),
    expectedEndRunProd_(ps.get<std::vector<int> >("expectedEndRunProd", default_)),
    expectedBeginSubRunProd_(ps.get<std::vector<int> >("expectedBeginSubRunProd", default_)),
    expectedEndSubRunProd_(ps.get<std::vector<int> >("expectedEndSubRunProd", default_)),

    expectedBeginRunNew_(ps.get<std::vector<int> >("expectedBeginRunNew", default_)),
    expectedEndRunNew_(ps.get<std::vector<int> >("expectedEndRunNew", default_)),
    expectedBeginLumiNew_(ps.get<std::vector<int> >("expectedBeginLumiNew", default_)),
    expectedEndLumiNew_(ps.get<std::vector<int> >("expectedEndLumiNew", default_)),

    expectedParents_(ps.get<std::vector<std::string> >("expectedParents", defaultvstring_)),

    expectedDroppedEvent_(ps.get<std::vector<int> >("expectedDroppedEvent", default_)),

    nRespondToOpenInputFile_(0),
    nRespondToCloseInputFile_(0),
    nRespondToOpenOutputFiles_(0),
    nRespondToCloseOutputFiles_(0),
    expectedRespondToOpenInputFile_(ps.get<int>("expectedRespondToOpenInputFile", -1)),
    expectedRespondToCloseInputFile_(ps.get<int>("expectedRespondToCloseInputFile", -1)),
    expectedRespondToOpenOutputFiles_(ps.get<int>("expectedRespondToOpenOutputFiles", -1)),
    expectedRespondToCloseOutputFiles_(ps.get<int>("expectedRespondToCloseOutputFiles", -1)),

    expectedInputFileNames_(ps.get<std::vector<std::string> >("expectedInputFileNames", defaultvstring_)),

    verbose_(ps.get<bool>("verbose", false)),

    index0_(0),
    index1_(0),
    index2_(0),
    index3_(0),
    index4_(0),
    index5_(0),
    index6_(0),
    index7_(0),
    parentIndex_(0)

  {
  }

  // -----------------------------------------------------------------

  TestMergeResults::~TestMergeResults()
  {
  }

  // -----------------------------------------------------------------

  void TestMergeResults::analyze(art::Event const & e, art::EventSetup const &)
  {
    if (verbose_) { mf::LogInfo("TestMergeResults") << "analyze"; }
    Run const & run = e.getRun();
    SubRun const & subRun = e.getSubRun();
    art::InputTag tag0("thingWithMergeProducer", "beginRun", "PROD");
    checkExpectedRunProducts(index0_, expectedBeginRunProd_, tag0, "analyze", run);
    art::InputTag tag1("thingWithMergeProducer", "beginRun");
    checkExpectedRunProducts(index4_, expectedBeginRunNew_, tag1, "analyze", run);
    art::InputTag tag2("thingWithMergeProducer", "endRun", "PROD");
    checkExpectedRunProducts(index1_, expectedEndRunProd_, tag2, "analyze", run);
    art::InputTag tag3("thingWithMergeProducer", "endRun");
    checkExpectedRunProducts(index5_, expectedEndRunNew_, tag3, "analyze", run);
    art::InputTag tag4("thingWithMergeProducer", "beginSubRun", "PROD");
    checkExpectedSubRunProducts(index2_, expectedBeginSubRunProd_, tag4, "analyze", subRun);
    art::InputTag tag5("thingWithMergeProducer", "beginSubRun");
    checkExpectedSubRunProducts(index6_, expectedBeginLumiNew_, tag5, "analyze", subRun);
    art::InputTag tag6("thingWithMergeProducer", "endSubRun", "PROD");
    checkExpectedSubRunProducts(index3_, expectedEndSubRunProd_, tag6, "analyze", subRun);
    art::InputTag tag7("thingWithMergeProducer", "endSubRun");
    checkExpectedSubRunProducts(index7_, expectedEndLumiNew_, tag7, "analyze", subRun);
    if (expectedDroppedEvent_.size() > 0) {
      art::InputTag tag("makeThingToBeDropped", "event", "PROD");
      e.getByLabel(tag, h_thingWithIsEqual);
      assert(h_thingWithIsEqual->a == expectedDroppedEvent_[0]);
      e.getByLabel(tag, h_thingWithMerge);
      assert(h_thingWithMerge.isValid());
    }
    // I'm not sure this test belongs in this module.  Originally it tested
    // merging of parentage for run and subRun products, but at some point the
    // parentage for run/subRun products stopped being written at all so there was
    // nothing to test.  This was the only real test of the provenance
    // parentage, so I just converted to a test of the parentage of products
    // in the Event rather than deleting it or writing a complete new test ...
    // It is actually convenient here, so maybe it is OK even if the module name
    // has nothing to do with this test.
    if (parentIndex_ < expectedParents_.size()) {
      art::InputTag tag("thingWithMergeProducer", "event", "PROD");
      e.getByLabel(tag, h_thing);
      std::string expectedParent = expectedParents_[parentIndex_];
      BranchID actualParentBranchID = h_thing.provenance()->parentage().parents()[0];
      // There ought to be a get that uses the BranchID as an argument, but
      // there is not at the moment so we get the Provenance first and use that
      // find the actual parent
      Provenance prov = e.getProvenance(actualParentBranchID);
      assert(expectedParent == prov.moduleLabel());
      art::InputTag tagparent(prov.moduleLabel(), prov.productInstanceName(), prov.processName());
      e.getByLabel(tagparent, h_thing);
      assert(h_thing->a == 11);
      ++parentIndex_;
    }
  }

  void TestMergeResults::beginRun(Run const & run, EventSetup const &)
  {
    index0_ += 3;
    index4_ += 3;
    index1_ += 3;
    index5_ += 3;
    if (verbose_) { mf::LogInfo("TestMergeResults") << "beginRun"; }
    art::InputTag tag("thingWithMergeProducer", "beginRun", "PROD");
    checkExpectedRunProducts(index0_, expectedBeginRunProd_, tag, "beginRun", run);
    art::InputTag tagnew("thingWithMergeProducer", "beginRun");
    checkExpectedRunProducts(index4_, expectedBeginRunNew_, tagnew, "beginRun", run);
    if (expectedDroppedEvent_.size() > 1) {
      art::InputTag tagd("makeThingToBeDropped", "beginRun", "PROD");
      run.getByLabel(tagd, h_thingWithIsEqual);
      assert(h_thingWithIsEqual->a == expectedDroppedEvent_[1]);
      run.getByLabel(tagd, h_thingWithMerge);
      assert(!h_thingWithMerge.isValid());
    }
  }

  void TestMergeResults::endRun(Run const & run, EventSetup const &)
  {
    index0_ += 3;
    index4_ += 3;
    index1_ += 3;
    index5_ += 3;
    if (verbose_) { mf::LogInfo("TestMergeResults") << "endRun"; }
    art::InputTag tag("thingWithMergeProducer", "endRun", "PROD");
    checkExpectedRunProducts(index1_, expectedEndRunProd_, tag, "endRun", run);
    art::InputTag tagnew("thingWithMergeProducer", "endRun");
    checkExpectedRunProducts(index5_, expectedEndRunNew_, tagnew, "endRun", run);
    if (expectedDroppedEvent_.size() > 2) {
      art::InputTag tagd("makeThingToBeDropped", "endRun", "PROD");
      run.getByLabel(tagd, h_thingWithIsEqual);
      assert(h_thingWithIsEqual->a == expectedDroppedEvent_[2]);
      run.getByLabel(tagd, h_thingWithMerge);
      assert(!h_thingWithMerge.isValid());
    }
  }

  void TestMergeResults::beginSubRun(SubRun const & subRun, EventSetup const &)
  {
    index2_ += 3;
    index6_ += 3;
    index3_ += 3;
    index7_ += 3;
    if (verbose_) { mf::LogInfo("TestMergeResults") << "beginSubRun"; }
    art::InputTag tag("thingWithMergeProducer", "beginSubRun", "PROD");
    checkExpectedSubRunProducts(index2_, expectedBeginSubRunProd_, tag, "beginSubRun", subRun);
    art::InputTag tagnew("thingWithMergeProducer", "beginSubRun");
    checkExpectedSubRunProducts(index6_, expectedBeginLumiNew_, tagnew, "beginSubRun", subRun);
    if (expectedDroppedEvent_.size() > 3) {
      art::InputTag tagd("makeThingToBeDropped", "beginSubRun", "PROD");
      subRun.getByLabel(tagd, h_thingWithIsEqual);
      assert(h_thingWithIsEqual->a == expectedDroppedEvent_[3]);
      subRun.getByLabel(tagd, h_thingWithMerge);
      assert(!h_thingWithMerge.isValid());
    }
  }

  void TestMergeResults::endSubRun(SubRun const & subRun, EventSetup const &)
  {
    index2_ += 3;
    index6_ += 3;
    index3_ += 3;
    index7_ += 3;
    if (verbose_) { mf::LogInfo("TestMergeResults") << "endSubRun"; }
    art::InputTag tag("thingWithMergeProducer", "endSubRun", "PROD");
    checkExpectedSubRunProducts(index3_, expectedEndSubRunProd_, tag, "endSubRun", subRun);
    art::InputTag tagnew("thingWithMergeProducer", "endSubRun");
    checkExpectedSubRunProducts(index7_, expectedEndLumiNew_, tagnew, "endSubRun", subRun);
    if (expectedDroppedEvent_.size() > 4) {
      art::InputTag tagd("makeThingToBeDropped", "endSubRun", "PROD");
      subRun.getByLabel(tagd, h_thingWithIsEqual);
      assert(h_thingWithIsEqual->a == expectedDroppedEvent_[4]);
      subRun.getByLabel(tagd, h_thingWithMerge);
      assert(!h_thingWithMerge.isValid());
    }
  }

  void TestMergeResults::respondToOpenInputFile(FileBlock const & fb)
  {
    index0_ += 3;
    index1_ += 3;
    index2_ += 3;
    index3_ += 3;
    index4_ += 3;
    index5_ += 3;
    index6_ += 3;
    index7_ += 3;
    if (verbose_) { mf::LogInfo("TestMergeResults") << "respondToOpenInputFile"; }
    if (!expectedInputFileNames_.empty()) {
      if (expectedInputFileNames_.size() <= static_cast<unsigned>(nRespondToOpenInputFile_) ||
          expectedInputFileNames_[nRespondToOpenInputFile_] != fb.fileName()) {
        std::cerr << "Error while testing merging of run/subRun products in TestMergeResults.cc\n"
                  << "Unexpected input filename, expected name = " << expectedInputFileNames_[nRespondToOpenInputFile_]
                  << "    actual name = " << fb.fileName() << std::endl;
        abort();
      }
    }
    ++nRespondToOpenInputFile_;
  }

  void TestMergeResults::respondToCloseInputFile(FileBlock const & fb)
  {
    if (verbose_) { mf::LogInfo("TestMergeResults") << "respondToCloseInputFile"; }
    ++nRespondToCloseInputFile_;
  }

  void TestMergeResults::respondToOpenOutputFiles(FileBlock const & fb)
  {
    if (verbose_) { mf::LogInfo("TestMergeResults") << "respondToOpenOutputFiles"; }
    ++nRespondToOpenOutputFiles_;
  }

  void TestMergeResults::respondToCloseOutputFiles(FileBlock const & fb)
  {
    if (verbose_) { mf::LogInfo("TestMergeResults") << "respondToCloseOutputFiles"; }
    ++nRespondToCloseOutputFiles_;
  }

  void TestMergeResults::endJob()
  {
    if (verbose_) { mf::LogInfo("TestMergeResults") << "endJob"; }
    if (expectedRespondToOpenInputFile_ > -1 && nRespondToOpenInputFile_ != expectedRespondToOpenInputFile_) {
      std::cerr << "Error while testing merging of run/subRun products in TestMergeResults.cc\n"
                << "Unexpected number of calls to the function respondToOpenInputFile" << std::endl;
      abort();
    }
    if (expectedRespondToCloseInputFile_ > -1 && nRespondToCloseInputFile_ != expectedRespondToCloseInputFile_) {
      std::cerr << "Error while testing merging of run/subRun products in TestMergeResults.cc\n"
                << "Unexpected number of calls to the function respondToCloseInputFile" << std::endl;
      abort();
    }
    if (expectedRespondToOpenOutputFiles_ > -1 && nRespondToOpenOutputFiles_ != expectedRespondToOpenOutputFiles_) {
      std::cerr << "Error while testing merging of run/subRun products in TestMergeResults.cc\n"
                << "Unexpected number of calls to the function respondToOpenOutputFiles" << std::endl;
      abort();
    }
    if (expectedRespondToCloseOutputFiles_ > -1 && nRespondToCloseOutputFiles_ != expectedRespondToCloseOutputFiles_) {
      std::cerr << "Error while testing merging of run/subRun products in TestMergeResults.cc\n"
                << "Unexpected number of calls to the function respondToCloseOutputFiles" << std::endl;
      abort();
    }
  }

  void
  TestMergeResults::checkExpectedRunProducts(unsigned int index,
      std::vector<int> const & expectedValues,
      InputTag const & tag,
      const char * functionName,
      Run const & run)
  {
    if ((index + 2) < expectedValues.size()) {
      int expected = expectedValues[index];
      if (expected != 0) {
        run.getByLabel(tag, h_thing);
        if (h_thing->a != expected)
        { abortWithMessage(functionName, "Thing", tag, expected, h_thing->a); }
      }
      expected = expectedValues[index + 1];
      if (expected != 0) {
        run.getByLabel(tag, h_thingWithMerge);
        if (h_thingWithMerge->a != expected)
        { abortWithMessage(functionName, "ThingWithMerge", tag, expected, h_thingWithMerge->a); }
      }
      expected = expectedValues[index + 2];
      if (expected != 0) {
        run.getByLabel(tag, h_thingWithIsEqual);
        if (h_thingWithIsEqual->a != expected)
        { abortWithMessage(functionName, "ThingWithIsEqual", tag, expected, h_thingWithIsEqual->a); }
      }
    }
  }

  void
  TestMergeResults::checkExpectedSubRunProducts(unsigned int index,
      std::vector<int> const & expectedValues,
      InputTag const & tag,
      const char * functionName,
      SubRun const & subRun)
  {
    if ((index + 2) < expectedValues.size()) {
      int expected = expectedValues[index];
      if (expected != 0) {
        subRun.getByLabel(tag, h_thing);
        if (h_thing->a != expected)
        { abortWithMessage(functionName, "Thing", tag, expected, h_thing->a); }
      }
      expected = expectedValues[index + 1];
      if (expected != 0) {
        subRun.getByLabel(tag, h_thingWithMerge);
        if (h_thingWithMerge->a != expected)
        { abortWithMessage(functionName, "ThingWithMerge", tag, expected, h_thingWithMerge->a); }
      }
      expected = expectedValues[index + 2];
      if (expected != 0) {
        subRun.getByLabel(tag, h_thingWithIsEqual);
        if (h_thingWithIsEqual->a != expected)
        { abortWithMessage(functionName, "ThingWithIsEqual", tag, expected, h_thingWithIsEqual->a); }
      }
    }
  }

  void TestMergeResults::abortWithMessage(const char * whichFunction, const char * type, art::InputTag const & tag,
                                          int expectedValue, int actualValue) const
  {
    std::cerr << "Error while testing merging of run/subRun products in TestMergeResults.cc\n"
              << "In function " << whichFunction << " looking for product of type " << type << "\n"
              << tag << "\n"
              << "Expected value = " << expectedValue << " actual value = " << actualValue << std::endl;
    abort();
  }
}

using arttest::TestMergeResults;

DEFINE_ART_MODULE(TestMergeResults);
