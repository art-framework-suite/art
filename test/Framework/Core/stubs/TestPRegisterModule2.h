#ifndef test_Framework_Core_stubs_TestPRegisterModule2_h
#define test_Framework_Core_stubs_TestPRegisterModule2_h
// -*- C++ -*-
//
// Package:     test
// Class  :     TestPRegisterModule2
//
/**\class TestPRegisterModule2 TestPRegisterModule2.h Framework/test/interface/TestPRegisterModule2.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Sat Sep 24 10:57:51 CEST 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/EDProducer.h"

// forward declarations

namespace art {
  class Event;
  class EventSetup;
  class ParameterSet;
}

class TestPRegisterModule2 : public art::EDProducer
{
public:
   explicit TestPRegisterModule2(fhicl::ParameterSet const& p);

   void produce(art::Event& e, art::EventSetup const&);

private:
};

#endif /* test_Framework_Core_stubs_TestPRegisterModule2_h */

// Local Variables:
// mode: c++
// End:
