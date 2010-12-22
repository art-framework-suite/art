#ifndef ServiceRegistry_ServiceHandle_h
#define ServiceRegistry_ServiceHandle_h
// -*- C++ -*-
//
// Package:     ServiceRegistry
// Class  :     Service
//
/*

 Description: Smart pointer used to give easy access to Services.

*/

#include "art/Framework/Services/Registry/ServiceRegistry.h"

namespace art {
   template<class T>
   class Service
{

   public:
   Service() {}
   //virtual ~Service();

   // ---------- const member functions ---------------------
   T* operator->() const {
      return &(ServiceRegistry::instance().template get<T>());
   }

   T& operator*() const {
      return ServiceRegistry::instance().template get<T>();
   }

   bool isAvailable() const {
      return ServiceRegistry::instance().template isAvailable<T>();
   }

   operator bool() const {
      return isAvailable();
   }

   private:
};

}

#endif
