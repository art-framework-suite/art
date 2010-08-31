
#include "art/Framework/Services/Registry/Service.h"
#include "art/Framework/Services/Basic/TFileService.h"
#include "art/ParameterSet/ParameterSet.h"

#include "test/Framework/Services/Basic/TestTFileServiceAnalyzer.h"

using namespace edm;
using namespace std;


TestTFileServiceAnalyzer::TestTFileServiceAnalyzer( const ParameterSet & cfg ) :
  dir1_( cfg.getParameter<string>( "dir1" ) ),
  dir2_( cfg.getParameter<string>( "dir2" ) ) {
  Service<TFileService> fs;
  if ( dir1_.empty() ) {
    h_test1 = fs->make<TH1F>( "test1"  , "test histogram #1", 100,  0., 100. );
  } else {
    TFileDirectory dir1 = fs->mkdir( dir1_ );
    h_test1 = dir1.make<TH1F>( "test1"  , "test histogram #1", 100,  0., 100. );
  }
  if ( dir2_.empty() ) {
    h_test2 = fs->make<TH1F>( "test2"  , "test histogram #2", 100,  0., 100. );
  } else {
    TFileDirectory dir2 = fs->mkdir( dir2_ );
    h_test2 = dir2.make<TH1F>( "test2"  , "test histogram #2", 100,  0., 100. );
  }
}

void TestTFileServiceAnalyzer::analyze( const Event& evt, const EventSetup& ) {
  h_test1->Fill( 50. );
  h_test2->Fill( 60. );
}
