// -*- C++ -*-
//
// Package:     PluginManager
// Class  :     PluginFactoryBase
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Wed Apr  4 13:09:50 EDT 2007
//
//

// system include files

// user include files
#include "art/Framework/PluginManager/PluginFactoryBase.h"
#include "art/Framework/PluginManager/PluginFactoryManager.h"

namespace artplugin {
//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//

// PluginFactoryBase::PluginFactoryBase(const PluginFactoryBase& rhs)
// {
//    // do actual copying here;
// }

PluginFactoryBase::~PluginFactoryBase()
{
}

//
// assignment operators
//
// const PluginFactoryBase& PluginFactoryBase::operator=(const PluginFactoryBase& rhs)
// {
//   //An exception safe implementation is
//   PluginFactoryBase temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
PluginFactoryBase::finishedConstruction()
{
   PluginFactoryManager::get()->addFactory(this);
}

void
PluginFactoryBase::newPlugin(const std::string& iName)
{
  PluginInfo info;
  info.loadable_=boost::filesystem::path(PluginManager::loadingFile(),boost::filesystem::no_check);
  info.name_=iName;
  newPluginAdded_(category(),info);
}
//
// const member functions
//

//
// static member functions
//
}
