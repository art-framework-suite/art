#include <cassert>
#include <fstream>
#include <initializer_list>
#include <map>
#include <sstream>
#include <string>

#include "TFile.h"
#include "TH1.h"

// This macro just checks the integrals for the histograms

int TestTFileService_r(std::string const& input)
{
  std::ifstream ifs{input};
  std::map<std::string, int> integrals_per_file;
  for (std::string line{}; std::getline(ifs, line);) {
    std::istringstream iss{line};
    std::string filename;
    int i;
    iss >> filename >> i;
    integrals_per_file.emplace(filename, i);
  }
  auto const histogram_names = {"a1/a/test1", "a1/b/test2", "a1/respondToOpenFile/test3"};

  for (auto const& pr : integrals_per_file) {
    auto const& filename = pr.first;
    auto const integral = pr.second;
    auto f = TFile::Open(filename.c_str());
    assert(f);

    for (auto const& histname : histogram_names) {
      auto h = dynamic_cast<TH1*>(f->Get(histname));
      assert(h);
      assert(h->Integral() == integral);
    }
  }
  return 0;
}
