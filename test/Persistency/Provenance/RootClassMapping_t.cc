#define BOOST_TEST_MODULE(RootClassMapping_t)
#include "boost/test/auto_unit_test.hpp"
#include "boost/test/output_test_stream.hpp"

using boost::test_tools::output_test_stream;

#include "RootClassMapping_t.h"

#include <iostream>
#include <memory>
#include <string>

#include "TClass.h"
#include "TBranch.h"
#include "TFile.h"
#include "TTree.h"

template <typename A, typename B>
void
adoptStreamerWithOS(std::ostream & os = std::cerr)
{
  static TClassRef cl(TClass::GetClass(typeid(TestProd<A, B>)));
  cl->AdoptStreamer(new TestProdStreamer<A, B>(os));
}

struct Initializer {
  Initializer() {
    TClass::GetClass(typeid(TestProd<size_t, std::string>))->SetCanSplit(0);
    TClass::GetClass(typeid(TestProd<std::string, size_t>))->SetCanSplit(0);
    adoptStreamerWithOS<std::string, size_t>();
    adoptStreamerWithOS<size_t, std::string>();
  }
};

struct TestFixture {
  TestFixture() {
    static Initializer init;
  }
};

BOOST_FIXTURE_TEST_SUITE(RootClassMapping_t, TestFixture)

BOOST_AUTO_TEST_CASE(write)
{
  TestProd<std::string, size_t> *tpOut = new TestProd<std::string, size_t>;
  tpOut->data.push_back(25);
  TFile tfOut("out.root", "RECREATE");
  TTree * treeOut = new TTree("T", "Test class mapping rules.");
  treeOut->Branch("b1", &tpOut);
  treeOut->Fill();
  tfOut.Write();
}

BOOST_AUTO_TEST_CASE(read)
{
  output_test_stream os;
  adoptStreamerWithOS<std::string, size_t>(os);
  adoptStreamerWithOS<size_t, std::string>(os);
  TestProd<size_t, std::string> tpIn;
  TestProd<size_t, std::string> *ptpIn = &tpIn;
  TFile tfIn("out.root");
  TTree * treeIn = (TTree *)tfIn.Get("T");
  treeIn->SetBranchAddress("b1", &ptpIn);
  treeIn->GetEntry(0);
  std::cerr << os.str();
  BOOST_CHECK(os.is_equal("Attempting to read stored object as a TestProd<unsigned long,string>.\n"));
  std::cerr
      << "Read data: "
      << tpIn.data.front()
      << ".\n";
}

BOOST_AUTO_TEST_SUITE_END()
