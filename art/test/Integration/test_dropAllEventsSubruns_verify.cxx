#include <cassert>
#include <iostream>
#include <string>

#include "TFile.h"
#include "TH1F.h"
#include "TTree.h"

using namespace std;

int
check_ttree(TFile const* f, char const* name, bool const empty)
{
  TTree* tree = nullptr;
  const_cast<TFile*>(f)->GetObject(name, tree);
  assert(tree != nullptr);
  if ((tree->GetEntries() == 0) && !empty) {
    cerr << "Tree " << name << " from " << f->GetName()
         << " not supposed to be empty.\n";
    return 1;
  }
  if ((tree->GetEntries() != 0) && empty) {
    cerr << "Tree " << name << " from " << f->GetName()
         << " supposed to be empty.\n"
         << " it has " << tree->GetEntries() << " entries.\n";
    return 1;
  }
  return 0;
}

int
test_dropAllEventsSubruns_verify(string const& suffix = "")
{
  bool const empty = true;
  bool const not_empty = false;
  string qual = suffix.empty() ? "" : string("_") + suffix;
  string filename = string("out_dropAllEvents") + qual + string(".root");
  TFile* f = TFile::Open(filename.c_str());
  if (f == nullptr) {
    return 1;
  }
  int passed = 0;
  passed += check_ttree(f, "Events", empty);
  passed += check_ttree(f, "EventMetaData", empty);
  passed += check_ttree(f, "SubRuns", not_empty);
  passed += check_ttree(f, "SubRunMetaData", not_empty);
  passed += check_ttree(f, "Runs", not_empty);
  passed += check_ttree(f, "RunMetaData", not_empty);
  filename = string("out_dropAllEventsSubruns1") + qual + string(".root");
  f = TFile::Open(filename.c_str());
  if (f == nullptr) {
    return 2;
  }
  passed += check_ttree(f, "Events", empty);
  passed += check_ttree(f, "EventMetaData", empty);
  passed += check_ttree(f, "SubRuns", empty);
  passed += check_ttree(f, "SubRunMetaData", empty);
  passed += check_ttree(f, "Runs", not_empty);
  passed += check_ttree(f, "RunMetaData", not_empty);
  filename = string("out_dropAllEventsSubruns2") + qual + string(".root");
  f = TFile::Open(filename.c_str());
  if (f == nullptr) {
    return 3;
  }
  passed += check_ttree(f, "Events", empty);
  passed += check_ttree(f, "EventMetaData", empty);
  passed += check_ttree(f, "SubRuns", empty);
  passed += check_ttree(f, "SubRunMetaData", empty);
  passed += check_ttree(f, "Runs", not_empty);
  passed += check_ttree(f, "RunMetaData", not_empty);
  return passed;
}

int
main(int argc, char** argv)
{
  if (argc > 1) {
    test_dropAllEventsSubruns_verify(string(argv[1]));
  } else {
    test_dropAllEventsSubruns_verify();
  }
}
