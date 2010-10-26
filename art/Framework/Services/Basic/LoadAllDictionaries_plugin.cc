//
// Package:     Services
// Class  :     LoadAllDictionaries
//


#include "art/Framework/Services/Basic/LoadAllDictionaries.h"

#include "art/Framework/PluginManager/PluginCapabilities.h"
#include "art/Framework/PluginManager/PluginManager.h"
#include "art/Framework/Services/Registry/ServiceMaker.h"

#include "Cintex/Cintex.h"
#include "fhiclcpp/ParameterSet.h"

using art::service::LoadAllDictionaries;


//
// constructors and destructor
//
art::service::LoadAllDictionaries::LoadAllDictionaries(const fhicl::ParameterSet& iConfig)
{
   bool doLoad(iConfig.get<bool>("doLoad",true));
   if(doLoad) {
     ROOT::Cintex::Cintex::Enable();

     artplugin::PluginManager*db =  artplugin::PluginManager::get();

     typedef artplugin::PluginManager::CategoryToInfos CatToInfos;

     CatToInfos::const_iterator itFound = db->categoryToInfos().find("Capability");

     if(itFound == db->categoryToInfos().end()) {
       return;
     }
     std::string lastClass;
     const std::string cPrefix("LCGReflex/");
     const std::string mystring("art::Wrapper");

     for (artplugin::PluginManager::Infos::const_iterator itInfo = itFound->second.begin(),
          itInfoEnd = itFound->second.end();
          itInfo != itInfoEnd; ++itInfo)
     {
       if (lastClass == itInfo->name_) {
         continue;
       }

       lastClass = itInfo->name_;
       if (lastClass.find(mystring) != std::string::npos) {
         artplugin::PluginCapabilities::get()->load(lastClass);
       }
       //NOTE: since we have the library already, we could be more efficient if we just load it ourselves
     }
   }
}


// ======================================================================


DEFINE_FWK_SERVICE_MAKER(LoadAllDictionaries,art::serviceregistry::ParameterSetMaker<LoadAllDictionaries>);
