// -*- C++ -*-
//
// Package:    TestFailuresAnalyzer
// Class:      TestFailuresAnalyzer
//
*/

#include "FWCore/Framework/test/stubs/TestFailuresAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "cetlib/exception.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

enum {
   kConstructor,
   kBeginOfJob,
   kEvent,
   kEndOfJob,
   kBeginOfJobBadXML
};
//
// constructors and destructor
//
TestFailuresAnalyzer::TestFailuresAnalyzer(const fhicl::ParameterSet& iConfig)
: whichFailure_(iConfig.get<int>("whichFailure"))
{
   //now do what ever initialization is needed
   if(whichFailure_ == kConstructor){
      throw cet::exception("Test")<<" constructor";
   }
}


TestFailuresAnalyzer::~TestFailuresAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
TestFailuresAnalyzer::beginJob(const art::EventSetup&)
{
   if(whichFailure_ == kBeginOfJob){
      throw cet::exception("Test") <<" beginJob";
   }
   if(whichFailure_ == kBeginOfJobBadXML){
      throw cet::exception("Test") <<" beginJob with <BAD> >XML<";
   }
}

void
TestFailuresAnalyzer::endJob()
{
   if(whichFailure_ == kEndOfJob){
      throw cet::exception("Test") <<" endJob";
   }
}


void
TestFailuresAnalyzer::analyze(const art::Event& /* iEvent */, const art::EventSetup& /* iSetup */)
{
   if(whichFailure_ == kEvent){
      throw cet::exception("Test") <<" event";
   }

}

//define this as a plug-in
DEFINE_ART_MODULE(TestFailuresAnalyzer)
