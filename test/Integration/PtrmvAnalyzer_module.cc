////////////////////////////////////////////////////////////////////////
// Class:       PtrmvAnalyzer
// Module Type: analyzer
// File:        PtrmvAnalyzer_module.cc
//
// Generated at Tue May 31 08:01:04 2011 by Chris Green using artmod
// from art v0_07_07.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/Ptr.h"
#include "art/Persistency/Common/PtrVector.h"

#include "cetlib/map_vector.h"

#include <string>

namespace arttest {
  class PtrmvAnalyzer;
}

class arttest::PtrmvAnalyzer : public art::EDAnalyzer {
public:
  explicit PtrmvAnalyzer(fhicl::ParameterSet const &p);
  virtual ~PtrmvAnalyzer();

  virtual void analyze(art::Event const &e);

private:
  std::string inputLabel_;
};


arttest::PtrmvAnalyzer::PtrmvAnalyzer(fhicl::ParameterSet const &p)
  :
  inputLabel_(p.get<std::string>("input_label"))
{
}

arttest::PtrmvAnalyzer::~PtrmvAnalyzer() {
}

void arttest::PtrmvAnalyzer::analyze(art::Event const &e) {
  // map_vector retrieval.
  art::Handle<cet::map_vector<std::string> > mv;
  assert(e.getByLabel(inputLabel_, mv));
  std::string const *item;
  item = mv->getOrNull(cet::map_vector_key(0));
  assert(*item == "ONE");
  item = mv->getOrNull(cet::map_vector_key(3));
  assert(*item == "TWO");
  item = mv->getOrNull(cet::map_vector_key(5));
  assert(*item == "THREE");
  item = mv->getOrNull(cet::map_vector_key(7));
  assert(*item == "FOUR");
  item = mv->getOrNull(cet::map_vector_key(9));
  assert(item == nullptr);

  // Ptr retrieval.
  art::Handle<art::Ptr<std::string> > ptr;
  assert(e.getByLabel(inputLabel_, ptr));
  assert(**ptr == "TWO");

  // PtrVectorRetrieval.
  art::Handle<art::PtrVector<std::string> > pv;
  assert(e.getByLabel(inputLabel_, pv));
  assert(*(*pv)[0] == "THREE");
  assert(*(*pv)[1] == "ONE");
  assert(*(*pv)[2] == "FOUR");
  assert(*(*pv)[3] == "TWO");
}

DEFINE_ART_MODULE(arttest::PtrmvAnalyzer);
