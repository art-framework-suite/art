// -*- C++ -*-
//
// Package:    Integration
// Class:      UseValueExampleAnalyzer
//
/**\class UseValueExampleAnalyzer UseValueExampleAnalyzer.cc FWCore/Integration/test/UseValueExampleAnalyzer.cc

Description: <one line class summary>

Implementation:
<Notes on implementation>
*/
//
// Original Author:  Chris D Jones
//         Created:  Thu Sep  8 03:55:42 EDT 2005
//
//
//


// system include files
#include <memory>
#include <iostream>

// user include files
#include "art/Framework/Core/EDAnalyzer.h"

#include "art/Framework/Core/ModuleMacros.h"


#include "FWCore/Integration/test/ValueExample.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

//
// class decleration
//

class UseValueExampleAnalyzer : public art::EDAnalyzer {
public:
   explicit UseValueExampleAnalyzer(const art::ParameterSet&);
   ~UseValueExampleAnalyzer();


   virtual void analyze(const art::Event&, const art::EventSetup&);
private:
      // ----------member data ---------------------------
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
UseValueExampleAnalyzer::UseValueExampleAnalyzer(const art::ParameterSet& /* iConfig */)
{
   //now do what ever initialization is needed

}


UseValueExampleAnalyzer::~UseValueExampleAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
UseValueExampleAnalyzer::analyze(const art::Event& /* iEvent */, const art::EventSetup& /* iSetup*/)
{
   std::cout<<" value from service "<< art::ServiceHandle<ValueExample>()->value()<<std::endl;
}

//define this as a plug-in
DEFINE_FWK_MODULE(UseValueExampleAnalyzer);

