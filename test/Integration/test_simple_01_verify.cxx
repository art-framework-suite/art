#include <assert.h>
#include <iostream>

#include "TDirectory.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TGraph.h"
#include "TGraphPolar.h"

#define nullptr 0


int test_simple_01_verify() {
  TFile *f = TFile::Open("tfile_output.root");
  if (f==nullptr) return 1;

  // Having obtained 'f', we never use it again. We use the global
  // variable gDirectory, because that's what the 'cd' calls
  // manipulate. Really.

  // Make sure we have a directory named by our module label, "hist".
  bool rc = gDirectory->cd("hist");
  if (!rc) return 1;

  // Make sure we have the subdirectory "a", containing the TH1F
  // histogram named "test1".
  rc = gDirectory->cd("a");
  if (!rc) return 2;

  TH1F* h1(nullptr);
  TH2F* h2(nullptr);
  TH1F* h3(nullptr);

  gDirectory->GetObject("test1", h1);
  if (h1 == nullptr)
    {
      std::cerr << "test1 not found\n";
      return 1;
    }

  h1 = nullptr;
  gDirectory->GetObject("z", h1);
  if (h1 != nullptr)
    {
      std::cerr << "z incorrectly found\n";
      return 1;
    }

  h2 = nullptr;
  gDirectory->GetObject("test1", h2);
  if (h2 != nullptr)
    {
      std::cerr << "test1 incorrectly identified as a TH2F\n";
      return 1;
    }

  // Make sure we have the subdirectory "b", containing the TH2F
  // histogram named "test2".
  rc = gDirectory->cd("../b");
  if (!rc) return 3;

  h2 == nullptr;
  gDirectory->GetObject("test2", h2);
  if (h2 == nullptr)
    {
      std::cerr << "test2 not found\n";
      return 3;
    }

  // Make sure we have the subdirectory "respondToOpenInputFile",
  // containing the TH1F histogrm named "test3"
  rc = gDirectory->cd("../respondToOpenInputFile");
  if (!rc) return 4;

  h3 == nullptr;
  gDirectory->GetObject("test3", h3);
  if (h3 == nullptr)
    {
      std::cerr << "test3 not found\n";
      return 4;
    }

  // Make sure the top-level directory contains a TGraph named
  // "graphAtTopLevel".
  rc = gDirectory->cd("/hist");
  if (!rc) return 5;
  TGraph* pgraph = nullptr;
  gDirectory->GetObject("graphAtTopLevel", pgraph);
  if (pgraph == nullptr)
    {
      std::cerr << "graphAtTopLevel not found\n";
      return 5;
    }
  std::string title = pgraph->GetTitle();
  if (title != "graph at top level")
    {
      std::cerr << "TGraph at top level not recovered with correct title\n";
      return 5;
    }

  // Make sure the direcotyr "b" contains a TGraph named
  // "graphInSubdirectory"
  rc = gDirectory->cd("b");
  if (!rc) return 6;
  TGraphPolar* ppolar = nullptr;
  gDirectory->GetObject("graphInSubdirectory", ppolar);
  if (ppolar == nullptr)
    {
      std::cerr << "graphInSubdirectory not found\n";
      return 6;
    }
  title = ppolar->GetTitle();
  if (title != "graph in subdirectory")
    {
      std::cerr << "TGraph in subdirectory not recovered with correct title\n";
      return 6;
    }

  return 0;
}

