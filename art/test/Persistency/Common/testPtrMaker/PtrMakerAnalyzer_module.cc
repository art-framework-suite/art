////////////////////////////////////////////////////////////////////////
// Class:       PtrMakerAnalyzer
// Plugin Type: analyzer (art v2_05_00)
// File:        PtrMakerAnalyzer_module.cc
//
// Generated at Wed Nov 23 21:29:25 2016 by Saba Sehrish using cetskelgen
// from cetlib version v1_21_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "canvas/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

class PtrMakerAnalyzer;


class PtrMakerAnalyzer : public art::EDAnalyzer {
public:
  typedef art::PtrVector<int> intptrvector_t;
 
  explicit PtrMakerAnalyzer(fhicl::ParameterSet const & p);
  // The compiler-generated destructor is fine for non-base
  // classes without bare pointers or other resource use.

  // Plugins should not be copied or assigned.
  PtrMakerAnalyzer(PtrMakerAnalyzer const &) = delete;
  PtrMakerAnalyzer(PtrMakerAnalyzer &&) = delete;
  PtrMakerAnalyzer & operator = (PtrMakerAnalyzer const &) = delete;
  PtrMakerAnalyzer & operator = (PtrMakerAnalyzer &&) = delete;

  // Required functions.
  void analyze(art::Event const & e) override;

private:
  std::string fInputLabel;
  int nvalues;

};


PtrMakerAnalyzer::PtrMakerAnalyzer(fhicl::ParameterSet const & p)
  : EDAnalyzer(p)
  , fInputLabel(p.get<std::string>("input_label"))
  , nvalues    (p.get<int> ("nvalues")) 
{}

void PtrMakerAnalyzer::analyze(art::Event const & e)
{
  std::cerr << "PtrMakerAnalyzer is running\n";
  art::Handle<intptrvector_t> h;
  e.getByLabel(fInputLabel, h);
  size_t sz = h->size();
    if( sz != (size_t)nvalues ) {
      throw cet::exception("SizeMismatch")
        << "Expected a PtrVector of size " << nvalues
        << " but the obtained size is " << sz
        << '\n';
    }

  int eid = e.id().event();

  //now check the values
  intptrvector_t local(*h);
  for (int i = 0; i < nvalues; ++i) {
    assert(*local[i] == eid+i);
  }
}

DEFINE_ART_MODULE(PtrMakerAnalyzer)
