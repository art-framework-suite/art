#include "RootClassMapping_t.h"

#include <iostream>
#include <memory>
#include <string>

#include "Cintex/Cintex.h"
#include "TClass.h"
#include "TBranch.h"
#include "TFile.h"
#include "TTree.h"

void write()
{
  TestProd<std::string, size_t> *tpOut = new TestProd<std::string, size_t>();
  tpOut->data.push_back(25);
  TFile tfOut("out.root", "RECREATE");
  TTree * treeOut = new TTree("T", "Test class mapping rules.");
  treeOut->Branch("b1", &tpOut);
  treeOut->Fill();
  tfOut.Write();
}

void read()
{
  TestProd<size_t, std::string> tpIn;
  TestProd<size_t, std::string> *ptpIn = &tpIn;
  TFile tfIn("out.root");
  TTree * treeIn = (TTree *)tfIn.Get("T");
  treeIn->SetBranchAddress("b1", &ptpIn);
  treeIn->GetEntry(0);
  std::cerr
      << "Read data: "
      << tpIn.data.front()
      << ".\n";
}

int main(int, char **)
{
  ROOT::Cintex::Cintex::Enable();
  TClass::GetClass(typeid(TestProd<size_t, std::string>))->SetCanSplit(0);
  TClass::GetClass(typeid(TestProd<std::string, size_t>))->SetCanSplit(0);
  write();
  read();
  return 0;
}
