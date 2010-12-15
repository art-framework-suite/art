#ifndef FWCore_Services_LoadAllDictionaries_h
#define FWCore_Services_LoadAllDictionaries_h

//
// Package:     Services
// Class  :     LoadAllDictionaries
//
/**\class LoadAllDictionaries LoadAllDictionaries.h FWCore/Services/interface/LoadAllDictionaries.h

 Description: Loads all Capability dictionaries

*/


#include "fhiclcpp/ParameterSet.h"


namespace art {
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
}  // namespace art

#endif  // FWCore_Services_LoadAllDictionaries_h
