#include "TH1.h"
#include "TH2.h"
#include "TGraph.h"
#include "TGraphPolar.h"

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "fhiclcpp/ParameterSet.h"
#include <string>

using namespace art;
using namespace std;


class TestTFileServiceAnalyzer
  : public art::EDAnalyzer
{
public:
  // constructor
  explicit TestTFileServiceAnalyzer( const fhicl::ParameterSet & );

private:

  void analyze( const art::Event& e);
  // histograms
  TH1F * h_test1 = nullptr;
  TH2F * h_test2 = nullptr;
  // graphs
  TGraph * g1 = nullptr;
  TGraphPolar * g2 = nullptr;
  // sub-directory name
  std::string dir1_;
  std::string dir2_;
};  // TestTFileServiceAnalyzer


TestTFileServiceAnalyzer::TestTFileServiceAnalyzer(fhicl::ParameterSet const& cfg ) : 
  EDAnalyzer( cfg ),
  dir1_("a"),
  dir2_("b")
{
  assert(!dir1_.empty());
  assert(!dir2_.empty());

  ServiceHandle<TFileService> fs;

  TFileDirectory dir1 = fs->mkdir( dir1_ );
  h_test1 = dir1.make<TH1F>( "test1", "test histogram #1", 100,  0., 100. );

  TFileDirectory dir2 = fs->mkdir( dir2_ );
  h_test2 = dir2.make<TH2F>( "test2", "test histogram #2", 100,  0., 100., 10, 0., 20. );

  g1 = fs->makeAndRegister<TGraph>("graphAtTopLevel", "graph at top level", 10);
  g2 = dir2.makeAndRegister<TGraphPolar>("graphInSubdirectory", "graph in subdirectory", 20);  
}

void TestTFileServiceAnalyzer::analyze( const Event& ) {
  h_test1->Fill( 50. );
  h_test2->Fill( 60., 3. );
}

DEFINE_ART_MODULE(TestTFileServiceAnalyzer)
