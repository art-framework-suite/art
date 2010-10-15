// -*- C++ -*-
//
// Package:    WhatsItAnalyzer
// Class:      WhatsItAnalyzer
//
/**\class WhatsItAnalyzer WhatsItAnalyzer.cc test/WhatsItAnalyzer/src/WhatsItAnalyzer.cc

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//
// Original Author:  Chris Jones
//         Created:  Fri Jun 24 19:13:25 EDT 2005
//
//
//


// system include files
#include <memory>
#include <iostream>

// user include files
#include "art/Framework/Core/EDAnalyzer.h"

#include "art/Framework/Core/MakerMacros.h"


#include "FWCore/Integration/test/WhatsIt.h"
#include "FWCore/Integration/test/GadgetRcd.h"

#include "art/Framework/Core/ESHandle.h"
#include "art/Framework/Core/EventSetup.h"
//
// class decleration
//

namespace arttest {

class WhatsItAnalyzer : public art::EDAnalyzer {
   public:
      explicit WhatsItAnalyzer(const art::ParameterSet&);
      ~WhatsItAnalyzer();


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
WhatsItAnalyzer::WhatsItAnalyzer(const art::ParameterSet& /*iConfig*/)
{
   //now do what ever initialization is needed

}


WhatsItAnalyzer::~WhatsItAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
WhatsItAnalyzer::analyze(const art::Event& /*iEvent*/, const art::EventSetup& iSetup)
{
   using namespace art;
   ESHandle<WhatsIt> pSetup;
   iSetup.get<GadgetRcd>().get(pSetup);

   std::cout <<"WhatsIt "<<pSetup->a<<std::endl;
}

}
using namespace arttest;
//define this as a plug-in
DEFINE_FWK_MODULE(WhatsItAnalyzer);
