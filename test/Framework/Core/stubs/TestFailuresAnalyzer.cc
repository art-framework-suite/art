// -*- C++ -*-
//
// Package:    TestFailuresAnalyzer
// Class:      TestFailuresAnalyzer
//
/**\class TestFailuresAnalyzer TestFailuresAnalyzer.cc stubs/TestFailuresAnalyzer/src/TestFailuresAnalyzer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Chris Jones
//         Created:  Fri Sep  2 13:54:17 EDT 2005
//
//
//


// system include files
#include <memory>

// user include files

#include "art/Framework/Core/MakerMacros.h"

#include "fhiclcpp/ParameterSet.h"

#include "FWCore/Framework/test/stubs/TestFailuresAnalyzer.h"

#include "cetlib/exception.h"

//
// class decleration
//

//
// constants, enums and typedefs
//

//
// static data member definitions
//

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
TestFailuresAnalyzer::TestFailuresAnalyzer(const art::ParameterSet& iConfig)
: whichFailure_(iConfig.getParameter<int>("whichFailure"))
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
DEFINE_FWK_MODULE(TestFailuresAnalyzer);
