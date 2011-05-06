// -*- C++ -*-
//
// Package:    WhatsItWatcherAnalyzer
// Class:      WhatsItWatcherAnalyzer
//


#include "FWCore/Integration/test/GadgetRcd.h"
#include "FWCore/Integration/test/WhatsIt.h"
#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ESHandle.h"
#include "art/Framework/Core/ESWatcher.h"
#include "art/Framework/Core/ModuleMacros.h"
#include <iostream>
#include <memory>

namespace arttest {

class WhatsItWatcherAnalyzer : public art::EDAnalyzer {
   public:
      explicit WhatsItWatcherAnalyzer(const fhicl::ParameterSet&);
      ~WhatsItWatcherAnalyzer();


      virtual void analyze(const art::Event&, const art::EventSetup&);
   private:
      // ----------member data ---------------------------
        void watch1(const GadgetRcd& );
        void watch2(const GadgetRcd& );

        art::ESWatcher<GadgetRcd> watch1_;
        art::ESWatcher<GadgetRcd> watch2_;
        art::ESWatcher<GadgetRcd> watchBool_;

};

//
// constructors and destructor
//
WhatsItWatcherAnalyzer::WhatsItWatcherAnalyzer(const fhicl::ParameterSet& /*iConfig*/):
  watch1_(this,&WhatsItWatcherAnalyzer::watch1),
  watch2_(std::bind(&WhatsItWatcherAnalyzer::watch2,this,_1)),
  watchBool_()
{
   //now do what ever initialization is needed

}


WhatsItWatcherAnalyzer::~WhatsItWatcherAnalyzer()
{

   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
WhatsItWatcherAnalyzer::analyze(const art::Event& /*iEvent*/, const art::EventSetup& iSetup)
{
   bool w1 = watch1_.check(iSetup);
   bool w2 = watch2_.check(iSetup);
   bool w3 = watchBool_.check(iSetup);
   assert(w1 == w2);
   assert(w2 == w3 );
}

void
WhatsItWatcherAnalyzer::watch1(const GadgetRcd& iRcd)
{
  art::ESHandle<arttest::WhatsIt> pSetup;
  iRcd.get(pSetup);

  std::cout <<"watch1: WhatsIt "<<pSetup->a<<" changed"<<std::endl;
}

void
WhatsItWatcherAnalyzer::watch2(const GadgetRcd& iRcd)
{
  art::ESHandle<WhatsIt> pSetup;
  iRcd.get(pSetup);

  std::cout <<"watch2: WhatsIt "<<pSetup->a<<" changed"<<std::endl;
}

}
using namespace arttest;
//define this as a plug-in
DEFINE_ART_MODULE(WhatsItWatcherAnalyzer);
