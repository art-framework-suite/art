#ifndef test_TestPRegisterModule1_h
#define test_TestPRegisterModule1_h
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
   explicit TestPRegisterModule1(art::ParameterSet const& p);
   void produce(art::Event& e, art::EventSetup const&);

private:
   art::ParameterSet pset_;
};


#endif
