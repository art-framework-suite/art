//
// Package:     Services
// Class  :     LoadAllDictionaries
//

#include "art/Utilities/LoadAllDictionaries.h"

// this all needs to go...
#include "art/Framework/PluginManager/PluginCapabilities.h"
#include "art/Framework/PluginManager/PluginManager.h"

#include "Cintex/Cintex.h"

//
// constructors and destructor
//
void art::loadAllDictionaries()
{
  ROOT::Cintex::Cintex::Enable();

  // all this need to be turned into LibraryManager stuff
  artplugin::PluginManager*db =  artplugin::PluginManager::get();
  typedef artplugin::PluginManager::CategoryToInfos CatToInfos;
  CatToInfos::const_iterator itFound = db->categoryToInfos().find("Capability");
  
  if(itFound == db->categoryToInfos().end()) return;

  std::string lastClass;
  const std::string cPrefix("LCGReflex/");
  const std::string mystring("art::Wrapper");

  for (artplugin::PluginManager::Infos::const_iterator itInfo = itFound->second.begin(),
	 itInfoEnd = itFound->second.end();
       itInfo != itInfoEnd; ++itInfo)
    {
      if (lastClass == itInfo->name_) continue;
      
      lastClass = itInfo->name_;
      if (lastClass.find(mystring) != std::string::npos) 
	{
	  artplugin::PluginCapabilities::get()->load(lastClass);
	}

      // NOTE: since we have the library already, 
      // we could be more efficient if we just load it ourselves
      }
}
