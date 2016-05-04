#ifndef art_test_Framework_Services_Basic_TestInitRootHandlers_h
#define art_test_Framework_Services_Basic_TestInitRootHandlers_h
// -*- C++ -*-
//
// Package:     Modules
// Class  :     TestInitRootHandlers
//
/**\class TestInitRootHandlers TestInitRootHandlers.h FWCore/Modules/src/TestInitRootHandlers.h

 Description: prints out what data is contained within an Event at that point in the path

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Mon Sep 19 11:49:35 CEST 2005
//

// system include files
#include <string>
#include <map>
#include <vector>

// user include files
#include "art/Framework/Core/EDAnalyzer.h"

// forward declarations

class TestInitRootHandlers : public art::EDAnalyzer {
public:
   explicit TestInitRootHandlers(const fhicl::ParameterSet&);
   ~TestInitRootHandlers();

   virtual void analyze(const art::Event&, const art::EventSetup&);
   virtual void endJob();

private:

   // ----------member data ---------------------------
   std::string indentation_;
   std::string verboseIndentation_;
   std::vector<std::string> moduleLabels_;
   bool        verbose_;
   int         evno_;
   std::map<std::string, int>  cumulates_;
};



#endif /* art_test_Framework_Services_Basic_TestInitRootHandlers_h */

// Local Variables:
// mode: c++
// End:
