//
// Package:     Services
// Class  :     LoadAllDictionaries


#include "art/Framework/Services/Basic/LoadAllDictionaries.h"

#include "art/Framework/PluginManager/PluginCapabilities.h"
#include "art/Framework/PluginManager/PluginManager.h"

#include "Cintex/Cintex.h"
#include "fhiclcpp/ParameterSet.h"


//
// constructors and destructor
//
edm::service::LoadAllDictionaries::LoadAllDictionaries(const fhicl::ParameterSet& iConfig)
{
   bool doLoad(iConfig.getBool("doLoad",true));
   if(doLoad) {
     ROOT::Cintex::Cintex::Enable();

     edmplugin::PluginManager*db =  edmplugin::PluginManager::get();

     typedef edmplugin::PluginManager::CategoryToInfos CatToInfos;

     CatToInfos::const_iterator itFound = db->categoryToInfos().find("Capability");

     if(itFound == db->categoryToInfos().end()) {
       return;
     }
     std::string lastClass;
     const std::string cPrefix("LCGReflex/");
     const std::string mystring("edm::Wrapper");

     for (edmplugin::PluginManager::Infos::const_iterator itInfo = itFound->second.begin(),
          itInfoEnd = itFound->second.end();
          itInfo != itInfoEnd; ++itInfo)
     {
       if (lastClass == itInfo->name_) {
         continue;
       }

       lastClass = itInfo->name_;
       if (lastClass.find(mystring) != std::string::npos) {
         edmplugin::PluginCapabilities::get()->load(lastClass);
       }
       //NOTE: since we have the library already, we could be more efficient if we just load it ourselves
     }
   }
}
