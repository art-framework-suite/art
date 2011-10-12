#ifndef test_Framework_Services_Registry_stubs_DummyService_h
#define test_Framework_Services_Registry_stubs_DummyService_h
// -*- C++ -*-
//
// Package:     ServiceRegistry
// Class  :     DummyService
//
/**\class DummyService DummyService.h FWCore/ServiceRegistry/test/stubs/DummyService.h

 Description: <one line class summary>

 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Mon Sep  5 19:51:59 EDT 2005
//
//

// system include files

// user include files
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

// forward declarations
namespace testserviceregistry {
  class DummyService {

  public:
    DummyService(const fhicl::ParameterSet &, art::ActivityRegistry &);
    virtual ~DummyService();

    // ---------- const member functions ---------------------
    int value() const { return value_; }

    bool beginJobCalled() const {return beginJobCalled_;}
    // ---------- static member functions --------------------

    // ---------- member functions ---------------------------

  private:
    void doOnBeginJob() { beginJobCalled_ = true;}
    DummyService(const DummyService &); // stop default

    const DummyService & operator=(const DummyService &); // stop default

    // ---------- member data --------------------------------
    int value_;
    bool beginJobCalled_;
  };
}

#endif /* test_Framework_Services_Registry_stubs_DummyService_h */

// Local Variables:
// mode: c++
// End:
