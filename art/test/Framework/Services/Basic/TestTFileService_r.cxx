#include <fstream>
#include <initializer_list>
#include <map>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TH1.h"

using namespace std;

// This macro just checks the integrals for the event-level histograms

int
TestTFileService_r(string const& input)
{
  ifstream ifs{input};
  map<string, int> integrals_per_file;
  for (string line; getline(ifs, line);) {
    istringstream iss{line};
    string filename;
    int i;
    iss >> filename >> i;
    integrals_per_file.emplace(filename, i);
  }
  auto const histogram_names = {"a1/a/test1", "a1/b/test2"};
  for (auto const& pr : integrals_per_file) {
    auto const& filename = pr.first;
    auto const integral = pr.second;
    auto f = TFile::Open(filename.c_str());
    if ((f == nullptr) || f->IsZombie()) {
      return 1;
    }
    for (auto const& histname : histogram_names) {
      auto h = dynamic_cast<TH1*>(f->Get(histname));
      if (h == nullptr) {
        return 2;
      }
      if (h->Integral() != integral) {
        return 3;
      }
      delete h;
      h = nullptr;
    }
    delete f;
    f = nullptr;
  }
  return 0;
}

int
main(int argc, char** argv)
{
  if (argc > 0) {
    TestTFileService_r(string(argv[1]));
  }
}
