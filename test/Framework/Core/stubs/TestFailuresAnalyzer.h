#ifndef Framework_TestFailuresAnalyzer_h
#define Framework_TestFailuresAnalyzer_h
// -*- C++ -*-
//
// Package:     test
// Class  :     TestFailuresAnalyzer
//
/**\class TestFailuresAnalyzer TestFailuresAnalyzer.h Framework/test/interface/TestFailuresAnalyzer.h

 Description: <one line class summary>

 Usage:
    <usage>

*/

#include "art/Framework/Core/EDAnalyzer.h"

class TestFailuresAnalyzer : public art::EDAnalyzer {
public:
   explicit TestFailuresAnalyzer(const art::ParameterSet&);
   ~TestFailuresAnalyzer();


   virtual void analyze(const art::Event&, const art::EventSetup&);

   virtual void beginJob(const art::EventSetup&);
   virtual void endJob();

private:
      int whichFailure_;
};


#endif
