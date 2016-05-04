#ifndef art_test_Framework_Core_stubs_TestBeginEndJobAnalyzer_h
#define art_test_Framework_Core_stubs_TestBeginEndJobAnalyzer_h

//
// Package:     test
// Class  :     TestBeginEndJobAnalyzer
//
/**\class TestBeginEndJobAnalyzer TestBeginEndJobAnalyzer.h Framework/test/interface/TestBeginEndJobAnalyzer.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Fri Sep  2 14:17:17 EDT 2005
//
//

// system include files
#include "art/Framework/Core/EDAnalyzer.h"
// user include files

// forward declarations
class TestBeginEndJobAnalyzer : public art::EDAnalyzer {
public:
   explicit TestBeginEndJobAnalyzer(const fhicl::ParameterSet&);
   ~TestBeginEndJobAnalyzer();


   virtual void analyze(const art::Event&, const art::EventSetup&);

   virtual void beginJob(art::EventSetup const&);
   virtual void endJob();
   virtual void beginRun(art::Run const&);
   virtual void endRun(art::Run const&);
   virtual void beginSubRun(art::SubRun const&);
   virtual void endSubRun(art::SubRun const&);

   static bool beginJobCalled;
   static bool endJobCalled;
   static bool beginRunCalled;
   static bool endRunCalled;
   static bool beginSubRunCalled;
   static bool endSubRunCalled;
   static bool destructorCalled;
private:
      // ----------member data ---------------------------
};


#endif /* art_test_Framework_Core_stubs_TestBeginEndJobAnalyzer_h */

// Local Variables:
// mode: c++
// End:
