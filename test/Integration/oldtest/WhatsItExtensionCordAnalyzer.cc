// -*- C++ -*-
//
// Package:    WhatsItExtensionCordAnalyzer
// Class:      WhatsItExtensionCordAnalyzer
//
/**\class WhatsItExtensionCordAnalyzer WhatsItExtensionCordAnalyzer.cc test/WhatsItExtensionCordAnalyzer/src/WhatsItExtensionCordAnalyzer.cc

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

#include "art/Framework/Core/ModuleMacros.h"


#include "FWCore/Integration/test/WhatsIt.h"
#include "FWCore/Integration/test/GadgetRcd.h"


//Here is the ExtensionCord/Outlet headers
#include "art/Framework/Core/ESOutlet.h"
#include "art/Utilities/ExtensionCord.h"

//
// class decleration
//

namespace arttest {

  class Last {
public:
    Last(const art::ExtensionCord<WhatsIt>& iCord): cord_(iCord) {}
    void doIt() {
      std::cout <<"WhatsIt "<<cord_->a<<std::endl;
    }
private:
    art::ExtensionCord<WhatsIt> cord_;
  };

  class Middle {
public:
    Middle(const art::ExtensionCord<WhatsIt>& iCord): last_(iCord) {}
    void doIt() {
      last_.doIt();
    }
private:
    Last last_;
  };

class WhatsItExtensionCordAnalyzer : public art::EDAnalyzer {
   public:
      explicit WhatsItExtensionCordAnalyzer(const art::ParameterSet&);
      ~WhatsItExtensionCordAnalyzer();


      virtual void analyze(const art::Event&, const art::EventSetup&);
   private:
      // ----------member data ---------------------------
        art::ExtensionCord<WhatsIt> cord_;
        Middle middle_;
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
WhatsItExtensionCordAnalyzer::WhatsItExtensionCordAnalyzer(const art::ParameterSet& /*iConfig*/) :
cord_(),
middle_(cord_)
{
   //now do what ever initialization is needed

}


WhatsItExtensionCordAnalyzer::~WhatsItExtensionCordAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
WhatsItExtensionCordAnalyzer::analyze(const art::Event& /*iEvent*/, const art::EventSetup& iSetup)
{
  art::ESOutlet<WhatsIt,GadgetRcd> outlet( iSetup, cord_ );

  middle_.doIt();
}

}
using namespace arttest;
//define this as a plug-in
DEFINE_FWK_MODULE(WhatsItExtensionCordAnalyzer);
