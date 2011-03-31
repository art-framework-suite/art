#ifndef test_Framework_Core_stubs_TestPRegisterModule1_h
#define test_Framework_Core_stubs_TestPRegisterModule1_h
// -*- C++ -*-
//
// Package:     test
// Class  :     TestPRegisterModule1
//
/**\class TestPRegisterModule1 TestPRegisterModule1.h Framework/test/interface/TestPRegisterModule1.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Sat Sep 24 10:57:48 CEST 2005
//
//

// system include files

// user include files
#include "art/Framework/Core/EDProducer.h"
#include "fhiclcpp/ParameterSet.h"

// forward declarations

class TestPRegisterModule1 : public art::EDProducer
{
public:
   explicit TestPRegisterModule1(fhicl::ParameterSet const& p);
   void produce(art::Event& e, art::EventSetup const&);

private:
   fhicl::ParameterSet pset_;
};


#endif /* test_Framework_Core_stubs_TestPRegisterModule1_h */

// Local Variables:
// mode: c++
// End:
