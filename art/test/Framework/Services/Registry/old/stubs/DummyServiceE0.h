#ifndef art_test_Framework_Services_Registry_old_stubs_DummyServiceE0_h
#define art_test_Framework_Services_Registry_old_stubs_DummyServiceE0_h
// -*- C++ -*-
//
// Package:     ServiceRegistry
// Class  :     DummyServiceE0
//
/**\class DummyServiceE0 DummyServiceE0.h FWCore/ServiceRegistry/test/stubs/DummyServiceE0.h

 Description: Used for tests of ServicesManager

 Usage:
    <usage>

*/
//
// Original Author:  David Dagenhart
//
//

#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"

namespace testserviceregistry {

  // The single extra letter in each of these typenames is intentionally
  // random.  These are used in a test of the ordering of service
  // calls, construction and destruction.  This previously
  // depended on the type and I want to be sure this is not
  // true anymore.

   class DummyServiceE0
   {
   public:
      DummyServiceE0(const fhicl::ParameterSet&,art::ActivityRegistry&);
      ~DummyServiceE0();
      void postBeginJob();
      void postEndJob();
   };

   class DummyServiceA1
   {
   public:
      DummyServiceA1(const fhicl::ParameterSet&,art::ActivityRegistry&);
      ~DummyServiceA1();
      void postBeginJob();
      void postEndJob();
   };

   class DummyServiceD2
   {
   public:
      DummyServiceD2(const fhicl::ParameterSet&,art::ActivityRegistry&);
      ~DummyServiceD2();
      void postBeginJob();
      void postEndJob();
   };

   class DummyServiceB3
   {
   public:
      DummyServiceB3(const fhicl::ParameterSet&,art::ActivityRegistry&);
      ~DummyServiceB3();
      void postBeginJob();
      void postEndJob();
   };

   class DummyServiceC4
   {
   public:
      DummyServiceC4(const fhicl::ParameterSet&,art::ActivityRegistry&);
      ~DummyServiceC4();
      void postBeginJob();
      void postEndJob();
   };
}

#endif /* art_test_Framework_Services_Registry_old_stubs_DummyServiceE0_h */

// Local Variables:
// mode: c++
// End:
