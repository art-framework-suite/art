#ifndef ServiceRegistry_ServiceWrapperBase_h
#define ServiceRegistry_ServiceWrapperBase_h
// -*- C++ -*-
//
// Package:     ServiceRegistry
// Class  :     ServiceWrapperBase
//
/**\class ServiceWrapperBase ServiceWrapperBase.h FWCore/ServiceRegistry/interface/ServiceWrapperBase.h

 Description: Base class through which the framework manages the lifetime of ServiceWrapper<T> objects.


*/
//
// Original Author:  Chris Jones
//         Created:  Mon Sep  5 13:33:01 EDT 2005
//
//

// system include files

// user include files

// forward declarations
namespace art {
   namespace serviceregistry {

      class ServiceWrapperBase
      {

public:
         ServiceWrapperBase();
         virtual ~ServiceWrapperBase();

         // ---------- const member functions ---------------------

         // ---------- static member functions --------------------

         // ---------- member functions ---------------------------

private:
         ServiceWrapperBase(const ServiceWrapperBase&); // stop default

         const ServiceWrapperBase& operator=(const ServiceWrapperBase&); // stop default

         // ---------- member data --------------------------------

      };
   }
}

#endif
