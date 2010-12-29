// -*- C++ -*-
//
// Package:     PluginManager
// Class  :     PluginCapabilities
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Fri Apr  6 12:36:24 EDT 2007
//
//

// system include files
#include <cstdlib>
#include <vector>
#include <string>

// user include files
#include "art/Framework/PluginManager/PluginCapabilities.h"
#include "art/Framework/PluginManager/SharedLibrary.h"
#include "art/Framework/PluginManager/PluginManager.h"
#include "art/Framework/PluginManager/FileOperations.h"
#include "cetlib/exception.h"
#include "art/Utilities/DebugMacros.h"

using namespace std;
using namespace art;
namespace fs = boost::filesystem;

namespace artplugin {

  std::string map_to_dict(const string& libname)
  {
    string map_name(libname);
    string sub("_map_");
    size_t found = map_name.find(sub);
    if(found==string::npos)
      {
        FDEBUG(1) << "Could not find _map_ file for " << map_name << "\n";
        throw cet::exception("NoMatch") << "could not find _dict_ library for "
                                      << libname << "\n";
      }
    string name(map_name.substr(0,found)+"_dict_plugin.so");
    return name;
  }

  PluginCapabilities::PluginCapabilities()
  {
    FDEBUG(2) << "PluginCapabilities ctor\n";
    typedef vector<string> vstring;

    vstring map_list;
    plugin::get_map_list(map_list);

    // for each library in list, load it, then find the magic symbol
    // SEAL_CAPABILITY, and load all its entries into our map

    vstring::iterator curr(map_list.begin()), end(map_list.end());
    for(;curr!=end;++curr)
      {
        FDEBUG(3) << "cap lib: " << *curr << "\n";
        const SharedLibrary& lib =
          PluginManager::get()->load(category(),*curr);
        FDEBUG(3) << "cap file: " << lib.path().native_file_string() << "\n";

        if(tryToFind(lib)==false)
          {
            throw cet::exception("missing symbol")
              << "cannot find symbol " << "SEAL_CAPABILITIES"
              << " in library " << *curr << "\n";
          }

        // JBK - temporary hack to just load all the libraries now for testing
        // the real demand loaded system is not working properly
        PluginManager::get()->load(category(),map_to_dict(lib.path().native_file_string()));
      }

    finishedConstruction();
  }

  // PluginCapabilities::PluginCapabilities(const PluginCapabilities& rhs)
  // {
  //    // do actual copying here;
  // }

  PluginCapabilities::~PluginCapabilities()
  {
  }

  //
  // assignment operators
  //
  // const PluginCapabilities& PluginCapabilities::operator=(const PluginCapabilities& rhs)
  // {
  //   //An exception safe implementation is
  //   PluginCapabilities temp(rhs);
  //   swap(rhs);
  //
  //   return *this;
  // }

  //
  // member functions
  //

  bool
  PluginCapabilities::tryToFind(const SharedLibrary& lib)
  {
    string name = map_to_dict(lib.path().native_file_string());

    FDEBUG(3) << "cap::tryToFind " << name << "\n";

    typedef void (*CapFunc)(const char **&, int&);
    const char** names;
    int size;
    const char* cap = "SEAL_CAPABILITIES";
    void* sym;
    CapFunc func;

    if(! lib.symbol(cap,sym)) return false;
    func=(CapFunc)sym;

    func(names, size);
    PluginInfo info;

    FDEBUG(3) << "cap: got " << size << " names from lib\n";

    for(int i=0;i<size;++i)
      {
        FDEBUG(5) << "  class name: " << name
                  << " : " << names[i] << "\n";
        // make class_name -> library_name mapping
        classes_[names[i]] = name;

        // announce to world that it is here!
        info.name_ = names[i];
        info.loadable_ = lib.path();
        this->newPluginAdded_(category(),info);
      }
    return true;
  }

  void
  PluginCapabilities::load(const std::string& iName)
  {
    FDEBUG(2) << "PluginCap::load: " << iName << "\n";
    if(tryToLoad(iName)==false)
      {
        throw cet::exception("missing symbol")
          << "cannot find symbol " << "SEAL_CAPABILITIES"
          << " in library " << iName << "\n";
      }
  }

  bool
  PluginCapabilities::tryToLoad(const std::string& class_name)
  {
    FDEBUG(3) << "tryToLoad(" << class_name << ")\n";
    ClassToLibMap::iterator i(classes_.find(class_name));
    if(i == classes_.end())
      {
        throw cet::exception("PluginNotFound")
          << "Cannot find plugin for " << class_name << "\n";
      }
    FDEBUG(3) << "tryToLoad: " << i->second << "\n";
    const SharedLibrary* lib =
      PluginManager::get()->tryToLoad(category(),i->second);

    FDEBUG(3) << "tryToLoad: " << lib << "\n";
    return (lib!=0);
  }

  //
  // const member functions
  //
  std::vector<PluginInfo>
  PluginCapabilities::available() const
  {
    FDEBUG(2) << "cap:available called\n";
    PluginInfo info;
    std::vector<PluginInfo> infos;
    infos.reserve(classes_.size());

    typedef std::map<std::string, boost::filesystem::path> PathMap;

    for(ClassToLibMap::const_iterator it = classes_.begin();
        it != classes_.end(); ++it)
      {
        info.name_ = it->first;
        info.loadable_ = it->second;
        infos.push_back(info);
      }
    return infos;
  }

  const std::string&
  PluginCapabilities::category() const
  {
    static const std::string s_cat("Capability");
    return s_cat;
  }

  //
  // static member functions
  //
  PluginCapabilities*
  PluginCapabilities::get() {
    static PluginCapabilities s_instance;
    return &s_instance;
  }

}  // artplugin
