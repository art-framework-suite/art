#include <assert.h>
#include <iostream>
#include <string>

#include "TH1F.h"
#include "TFile.h"
#include "TTree.h"

#define nullptr 0

int check_ttree(TFile const* f, char const* name, bool const empty )
{
  TTree* tree(nullptr);
  f->GetObject( name, tree );
  assert( tree != nullptr );
  if ( tree->GetEntries() == 0 && !empty ){
    std::cerr << "Tree " << name << " from " << f->GetName() << " not supposed to be empty." << std::endl;
    return 1;
  }
  if ( tree->GetEntries() !=0 && empty ) {
    std::cerr << "Tree " << name <<  " from " << f->GetName() << " supposed to be empty.\n"
              << " it has " << tree->GetEntries() << " entries."
              << std::endl;
    return 1;
  }
  return 0;
}

int test_dropAllEventsSubruns_verify(std::string const& suffix = "")
{

  bool const empty     = true;
  bool const not_empty = false;

  std::string qual = suffix.empty() ? "" : std::string("_")+suffix;

  std::string filename = std::string("out_dropAllEvents")+qual+std::string(".root");
  TFile *f = TFile::Open( filename.c_str() );
  if (f==nullptr) return 1;

  int passed(0);
  passed += check_ttree( f, "Events"        , empty);
  passed += check_ttree( f, "EventMetaData" , empty);
  passed += check_ttree( f, "SubRuns"       , not_empty);
  passed += check_ttree( f, "SubRunMetaData", not_empty);
  passed += check_ttree( f, "Runs"          , not_empty);
  passed += check_ttree( f, "RunMetaData"   , not_empty);

  filename = std::string("out_dropAllEventsSubruns1")+qual+std::string(".root");
  f = TFile::Open( filename.c_str() );
  if (f==nullptr) return 2;

  passed += check_ttree( f, "Events"        , empty);
  passed += check_ttree( f, "EventMetaData" , empty);
  passed += check_ttree( f, "SubRuns"       , empty);
  passed += check_ttree( f, "SubRunMetaData", empty);
  passed += check_ttree( f, "Runs"          , not_empty);
  passed += check_ttree( f, "RunMetaData"   , not_empty);

  filename = std::string("out_dropAllEventsSubruns2")+qual+std::string(".root");
  f = TFile::Open( filename.c_str() );
  if (f==nullptr) return 3;

  passed += check_ttree( f, "Events"        , empty);
  passed += check_ttree( f, "EventMetaData" , empty);
  passed += check_ttree( f, "SubRuns"       , empty);
  passed += check_ttree( f, "SubRunMetaData", empty);
  passed += check_ttree( f, "Runs"          , not_empty);
  passed += check_ttree( f, "RunMetaData"   , not_empty);

  return passed;

}

