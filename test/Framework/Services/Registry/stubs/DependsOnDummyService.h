#ifndef test_Framework_Services_Registry_stubs_DependsOnDummyService_h
#define test_Framework_Services_Registry_stubs_DependsOnDummyService_h
// -*- C++ -*-
//
// Package:     ServiceRegistry
// Class  :     DependsOnDummyService
//
/**\class DependsOnDummyService DependsOnDummyService.h FWCore/ServiceRegistry/test/stubs/DependsOnDummyService.h

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

// forward declarations
namespace testserviceregistry {
  class DependsOnDummyService {

  public:
    DependsOnDummyService();
    virtual ~DependsOnDummyService();

    // ---------- const member functions ---------------------
    int value() const { return value_; }

    // ---------- static member functions --------------------

    // ---------- member functions ---------------------------

  private:
    DependsOnDummyService(const DependsOnDummyService &); // stop default

    const DependsOnDummyService & operator=(const DependsOnDummyService &); // stop default

    // ---------- member data --------------------------------
    int value_;
  };
}

#endif /* test_Framework_Services_Registry_stubs_DependsOnDummyService_h */

// Local Variables:
// mode: c++
// End:
