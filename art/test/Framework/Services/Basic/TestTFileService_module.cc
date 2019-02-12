#include "TGraph.h"
#include "TGraphPolar.h"
#include "TH1.h"
#include "TH2.h"

#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/SharedAnalyzer.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

using namespace art;
using namespace std;

class TestTFileService : public art::SharedAnalyzer {
public:
  struct Config {};
  using Parameters = Table<Config>;
  explicit TestTFileService(Parameters const&, ProcessingFrame const& frame);

private:
  void analyze(Event const& e, ProcessingFrame const&) override;
  void respondToOpenInputFile(FileBlock const&,
                              ProcessingFrame const& frame) override;
  void setRootObjects();

  // histograms
  TH1F* h_test1{nullptr};
  TH2F* h_test2{nullptr};
  TH1F* h_test3{nullptr};

  // graphs
  TGraph* g1{nullptr};
  TGraphPolar* g2{nullptr};

  // sub-directory name
  std::string dir1_{"a"};
  std::string dir2_{"b"};
  std::string dir3_{"respondToOpenInputFile"};

}; // TestTFileService

TestTFileService::TestTFileService(Parameters const& p,
                                   ProcessingFrame const& frame)
  : SharedAnalyzer{p}
{
  serialize(TFileService::resource_name());
  auto const fs = frame.serviceHandle<TFileService>();
  fs->registerFileSwitchCallback(this, &TestTFileService::setRootObjects);
  setRootObjects();
}

void
TestTFileService::setRootObjects()
{
  ServiceHandle<TFileService> fs;
  TFileDirectory dir1 = fs->mkdir(dir1_);
  h_test1 = dir1.make<TH1F>("test1", "test histogram #1", 100, 0., 100.);

  TFileDirectory dir2 = fs->mkdir(dir2_);
  h_test2 =
    dir2.make<TH2F>("test2", "test histogram #2", 100, 0., 100., 10, 0., 20.);

  TFileDirectory dir3 = fs->mkdir(dir3_);
  h_test3 = dir3.make<TH1F>("test3", "test respondToInputFile", 100, 0., 10.);

  g1 = fs->makeAndRegister<TGraph>("graphAtTopLevel", "graph at top level", 10);
  g2 = dir2.makeAndRegister<TGraphPolar>(
    "graphInSubdirectory", "graph in subdirectory", 20);
}

void
TestTFileService::respondToOpenInputFile(FileBlock const&,
                                         ProcessingFrame const&)
{
  h_test3->Fill(3.);
}

void
TestTFileService::analyze(Event const&, ProcessingFrame const&)
{
  h_test1->Fill(50.);
  h_test2->Fill(60., 3.);
}

DEFINE_ART_MODULE(TestTFileService)
