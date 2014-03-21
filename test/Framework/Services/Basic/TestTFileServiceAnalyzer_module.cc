#include "TH1.h"
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
  // process one event
  void analyze( const art::Event& , const art::EventSetup& );
  // histograms
  TH1F * h_test1, * h_test2;
  // sub-directory name
  std::string dir1_, dir2_;
};  // TestTFileServiceAnalyzer


TestTFileServiceAnalyzer::TestTFileServiceAnalyzer( const ParameterSet & cfg )
: EDAnalyzer( cfg )
, dir1_( cfg.get<string>( "dir1" ) )
, dir2_( cfg.get<string>( "dir2" ) )
{
  ServiceHandle<TFileService> fs;
  if ( dir1_.empty() ) {
    h_test1 = fs->make<TH1F>( "test1" , "test histogram #1", 100,  0., 100. );
  } else {
    TFileDirectory dir1 = fs->mkdir( dir1_ );
    h_test1 = dir1.make<TH1F>( "test1", "test histogram #1", 100,  0., 100. );
  }
  if ( dir2_.empty() ) {
    h_test2 = fs->make<TH1F>( "test2" , "test histogram #2", 100,  0., 100. );
  } else {
    TFileDirectory dir2 = fs->mkdir( dir2_ );
    h_test2 = dir2.make<TH1F>( "test2", "test histogram #2", 100,  0., 100. );
  }
}

void TestTFileServiceAnalyzer::analyze( const Event& evt, const EventSetup& ) {
  h_test1->Fill( 50. );
  h_test2->Fill( 60. );
}

DEFINE_ART_MODULE(TestTFileServiceAnalyzer)
