// -*- C++ -*-
//
// Package:     ParameterSet
// Class  :     edmParamDesc
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Thu Aug  2 13:33:53 EDT 2007
//
//

// system include files
#include <iostream>
#include <utility>
#include <cstdlib>
#include <string>
#include <memory>

// user include files
#include "art/Utilities/Exception.h"
#include "art/ParameterSet/ParameterSetDescriptionFillerPluginFactory.h"
#include "art/ParameterSet/ParameterSetDescriptionFillerBase.h"
#include "art/ParameterSet/ParameterSetDescription.h"

#include "art/Framework/PluginManager/PluginManager.h"
#include "art/Framework/PluginManager/standard.h"

static void print(const edm::ParameterSetDescription& iDesc,
                  const std::string& iIndent,
                  const std::string& iDelta)
{
  if(iDesc.isUnknown()) {
    std::cout <<iIndent<<"UNKNOWN\n";
  }else  {
    for(edm::ParameterSetDescription::parameter_const_iterator it = iDesc.parameter_begin(),
        itEnd = iDesc.parameter_end();
        it != itEnd;
        ++it) {
      std::cout <<iIndent
      <<((*it)->isTracked()?"":"untracked ")
      <<edm::parameterTypeEnumToString((*it)->type())
      <<" "
      <<(*it)->label()<<"\n";
    }
  }
}

int main (int argc, char **argv)
{
  try {

    edmplugin::PluginManager::configure(edmplugin::standard::config());

    typedef edmplugin::PluginManager::CategoryToInfos CatToInfos;

    const CatToInfos& catToInfos = edmplugin::PluginManager::get()->categoryToInfos();


    edm::ParameterSetDescriptionFillerPluginFactory* factory = edm::ParameterSetDescriptionFillerPluginFactory::get();

    CatToInfos::const_iterator itPlugins = catToInfos.find(factory->category());

    if(itPlugins == catToInfos.end() ) {
      return 0;
    }
    std::vector<edmplugin::PluginInfo> infos = itPlugins->second;
    std::string lastName;
    for(std::vector<edmplugin::PluginInfo>::const_iterator it = infos.begin(), itEnd=infos.end();
        it != itEnd;
        ++it) {
      if(lastName != it->name_) {
        lastName = it->name_;
        std::cout <<it->name_<<"\n";
        try {
          std::auto_ptr<edm::ParameterSetDescriptionFillerBase> filler(factory->create(it->name_));
          edm::ParameterSetDescription desc;
          filler->fill(desc, std::string());
          print(desc,"  "," ");
        }catch(const cms::Exception& e) {
          std::cout <<"  FAILED: could not read parameter info because \n"
          <<"   "<<e.what();
        }
        catch(const std::exception& e) {
          std::cout <<"  FAILED: could not read parameter info because \n"
          <<"   "<<e.what();
        }
        std::cout<<std::endl;
      }
    }
  } catch(const cms::Exception& e) {
    std::cerr<<"The following problem occurred\n"<<e.what()<<std::endl;
  } catch(const std::exception& e) {
  std::cerr<<"The following problem occurred\n"<<e.what()<<std::endl;
  }
  return 0;
}

