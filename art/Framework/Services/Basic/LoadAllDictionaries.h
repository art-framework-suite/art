#ifndef FWCore_Services_LoadAllDictionaries_h
#define FWCore_Services_LoadAllDictionaries_h

//
// Package:     Services
// Class  :     LoadAllDictionaries
//
/**\class LoadAllDictionaries LoadAllDictionaries.h FWCore/Services/interface/LoadAllDictionaries.h

 Description: Loads all Capability dictionaries

*/


#include "fhicl/ParameterSet.h"


namespace edm {
  namespace service {

    class LoadAllDictionaries
    {
    public:
      LoadAllDictionaries(const fhicl::ParameterSet&);

    private:
      // no copying
      LoadAllDictionaries(const LoadAllDictionaries&);
      const LoadAllDictionaries& operator=(const LoadAllDictionaries&);

    };


  }  // namespace service
}  // namespace edm

#endif  // FWCore_Services_LoadAllDictionaries_h
