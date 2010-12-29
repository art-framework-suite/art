// -*- C++ -*-
//
// Package:     PluginManager
// Class  :     PluginManager
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Wed Apr  4 14:28:58 EDT 2007
//
//

// system include files
#include <boost/bind.hpp>
#include <boost/mem_fn.hpp>

#include <boost/filesystem/operations.hpp>

#include <fstream>
#include <sstream>
#include <set>

// user include files
#include "art/Framework/PluginManager/PluginManager.h"
#include "art/Framework/PluginManager/PluginFactoryBase.h"
#include "art/Framework/PluginManager/PluginFactoryManager.h"
#include "art/Framework/PluginManager/CacheParser.h"
#include "cetlib/exception.h"
#include "art/Utilities/DebugMacros.h"

#include "art/Framework/PluginManager/standard.h"

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
  using namespace art;

  std::string pluginName(const std::string& name)
  {
    if(name[0]=='l' && name[1]=='i' && name[2]=='b') return name;

    std::ostringstream ost;
    ost << "lib" << name << "_plugin.so";
    return ost.str();
  }

PluginManager::PluginManager(const PluginManager::Config& iConfig) :
  searchPath_( iConfig.searchPath() )
{
    const boost::filesystem::path kCacheFile(standard::cachefileName());
    //NOTE: This may not be needed :/
    PluginFactoryManager* pfm = PluginFactoryManager::get();
    pfm->newFactory_.connect(boost::bind(boost::mem_fn(&PluginManager::newFactory),this,_1));

    // When building a single big executable the plugins are already registered in the
    // PluginFactoryManager, we therefore only need to populate the categoryToInfos_ map
    // with the relevant information.
    for (PluginFactoryManager::const_iterator i = pfm->begin(), e = pfm->end(); i != e; ++i)
    {
        categoryToInfos_[(*i)->category()] = (*i)->available();
    }

    //read in the files
    //Since we are looping in the 'precidence' order then the lists in categoryToInfos_ will also be
    // in that order
    std::set<std::string> alreadySeen;
    for(SearchPath::const_iterator itPath=searchPath_.begin(), itEnd = searchPath_.end();
        itPath != itEnd;
        ++itPath) {
      //take care of the case where the same path is passed in multiple times
      if (alreadySeen.find(*itPath) != alreadySeen.end() ) {
        continue;
      }
      alreadySeen.insert(*itPath);
      boost::filesystem::path dir(*itPath,boost::filesystem::no_check);
      if( exists( dir) ) {
        if(not is_directory(dir) ) {
          throw cet::exception("PluginManagerBadPath") <<"The path '"<<dir.native_file_string()<<"' for the PluginManager is not a directory";
        }
        boost::filesystem::path cacheFile = dir/kCacheFile;

        if(exists(cacheFile) ) {
          std::ifstream file(cacheFile.native_file_string().c_str());
          if(not file) {
            throw cet::exception("PluginMangerCacheProblem")<<"Unable to open the cache file '"<<cacheFile.native_file_string()
            <<"'. Please check permissions on file";
          }
          CacheParser::read(file, dir, categoryToInfos_);
        }
      }
    }
    //Since this should not be called until after 'main' has started, we can set the value
    loadingLibraryNamed_()="<loaded by another plugin system>";
}

// PluginManager::PluginManager(const PluginManager& rhs)
// {
//    // do actual copying here;
// }

PluginManager::~PluginManager()
{
}

//
// assignment operators
//
// const PluginManager& PluginManager::operator=(const PluginManager& rhs)
// {
//   //An exception safe implementation is
//   PluginManager temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//
void
PluginManager::newFactory(const PluginFactoryBase* i)
{
  FDEBUG(2) << "pluginman: newFactory " << i->category() << "\n";
  std::vector<PluginInfo> infos = i->available();
  categoryToInfos_[i->category()] = infos;

  std::vector<PluginInfo>::iterator it(infos.begin()),end(infos.end());
  for(;it!=end;++it)
    {
      FDEBUG(4) << "  info: " << it->name_ << "\n";
    }
}
//
// const member functions
//
namespace {
  struct PICompare {
    bool operator()(const PluginInfo& iLHS,
                    const PluginInfo& iRHS) const {
                      return iLHS.name_ < iRHS.name_;
                    }
  };
}

const boost::filesystem::path
PluginManager::loadableFor(const std::string& iCategory,
                             const std::string& iPlugin)
{
  bool throwIfFail = true;
  FDEBUG(2) << "loadableFor: Plugin name = " << iPlugin << "\n";
  return boost::filesystem::path(pluginName(iPlugin),boost::filesystem::no_check);
  //return loadableFor_(iCategory, iPlugin,throwIfFail);
}

const boost::filesystem::path&
PluginManager::loadableFor_(const std::string& iCategory,
                            const std::string& iPlugin,
                            bool& ioThrowIfFailElseSucceedStatus)
{
  // JBK changed
  FDEBUG(2) << "loadableFor_: Plugin name = " << iPlugin << "\n";

#if 0
  const bool throwIfFail = ioThrowIfFailElseSucceedStatus;
  ioThrowIfFailElseSucceedStatus = true;

  CategoryToInfos::iterator itFound = categoryToInfos_.find(iCategory);

  if(itFound == categoryToInfos_.end())
    {
      if(throwIfFail)
        {
          throw cet::exception("PluginNotFound")
            << "Unable to find plugin '"<<iPlugin
            << "' because the category '"<<iCategory<<"' has no known plugins";
        }
      else
        {
          ioThrowIfFailElseSucceedStatus = false;
          static boost::filesystem::path s_path;
          return s_path;
        }
    }

  PluginInfo i;
  i.name_ = iPlugin;
  typedef std::vector<PluginInfo>::iterator PIItr;

  std::pair<PIItr,PIItr> range = std::equal_range(itFound->second.begin(),
                                                  itFound->second.end(),
                                                  i,
                                                  PICompare() );

  if(range.first == range.second)
    {
      if(throwIfFail)
        {
          throw cet::exception("PluginNotFound")
            <<"Unable to find plugin '"<<iPlugin
            <<"'. Please check spelling of name.";
        }
      else
        {
          ioThrowIfFailElseSucceedStatus = false;
          static boost::filesystem::path s_path;
          return s_path;
        }
    }

  if(range.second - range.first > 1 )
    {
      //see if the come from the same directory
      if(range.first->loadable_.branch_path() == (range.first+1)->loadable_.branch_path())
        {
          //std::cout<<range.first->name_ <<" " <<(range.first+1)->name_<<std::endl;
          throw cet::exception("MultiplePlugins")
            <<"The plugin '"<<iPlugin<<"' is found in multiple files \n"
            " '"<<range.first->loadable_.leaf()<<"'\n '"
            <<(range.first+1)->loadable_.leaf()<<"'\n"
            "in directory '"
            <<range.first->loadable_.branch_path().native_file_string()<<"'.\n"
            " The code must be changed so the plugin only "
            " appears in one plugin file. "
            " You will need to remove the macro which registers the plugin"
            " so it only appears in"
            " one of these files.\n"
            " If none of these files register such a plugin, "
            " then the problem originates in a library to which all these"
            " files link.\n"
            " The plugin registration must be removed from that library "
            " since plugins are not allowed in regular libraries.";
        }
    }

  return range.first->loadable_;
#endif
}

namespace {
  class Sentry {
public:
    Sentry( std::string& iPath, const std::string& iNewPath):
      path_(iPath),
      oldPath_(iPath)
    {
      path_ = iNewPath;
    }
    ~Sentry() {
      path_ = oldPath_;
    }
private:
      std::string& path_;
      std::string oldPath_;
  };
}



const SharedLibrary& PluginManager::load(const std::string& iCategory,
                                         const std::string& iPlugin)
{
  return *tryToLoad(iCategory, iPlugin);

#if 0
  askedToLoadCategoryWithPlugin_(iCategory,iPlugin);
  const boost::filesystem::path& p = loadableFor(iCategory,iPlugin);

  //have we already loaded this?
  std::map<boost::filesystem::path, SharedLibPtr >::iterator itLoaded =
    loadables_.find(p);

  if(itLoaded == loadables_.end())
    {
      //try to make one
      goingToLoad_(p);
      Sentry s(loadingLibraryNamed_(), p.native_file_string());
      //boost::filesystem::path native(p.native_file_string(),boost::filesystem::no_check);
      boost::shared_ptr<SharedLibrary> ptr( new SharedLibrary(p) );
      loadables_[p]=ptr;
      justLoaded_(*ptr);
      return *ptr;
    }
  return *(itLoaded->second);
#endif
}

const SharedLibrary*
PluginManager::tryToLoad(const std::string& iCategory,
                         const std::string& iPlugin)
{
  askedToLoadCategoryWithPlugin_(iCategory,iPlugin);

  // JBK - temporary hack for cache
  typedef boost::shared_ptr<SharedLibrary> SharedLibPtr;
  typedef std::map<std::string, SharedLibPtr > LibMap;
  static  LibMap already_loaded;

  // JBK changes go here!!!
  LibMap::const_iterator it = already_loaded.find(iPlugin);

  if(it!=already_loaded.end())
    {
      return (it->second).get();
    }

  std::string new_name = pluginName(iPlugin);
  SharedLibPtr ptr( new SharedLibrary(new_name) );
  already_loaded[new_name] = ptr;
  return ptr.get();

#if 0
  bool ioThrowIfFailElseSucceedStatus = false;
  const boost::filesystem::path& p =
    loadableFor_(iCategory,iPlugin, ioThrowIfFailElseSucceedStatus);

  if( not ioThrowIfFailElseSucceedStatus ) {
    return 0;
  }

  //have we already loaded this?
  std::map<boost::filesystem::path, boost::shared_ptr<SharedLibrary> >::iterator itLoaded =
    loadables_.find(p);
  if(itLoaded == loadables_.end()) {
    //try to make one
    goingToLoad_(p);
    Sentry s(loadingLibraryNamed_(), p.native_file_string());
    //boost::filesystem::path native(p.native_file_string(),boost::filesystem::no_check);
    boost::shared_ptr<SharedLibrary> ptr( new SharedLibrary(p) );
    loadables_[p]=ptr;
    justLoaded_(*ptr);
    return ptr.get();
  }
  return (itLoaded->second).get();
#endif
}

//
// static member functions
//
PluginManager*
PluginManager::get()
{
  PluginManager* manager = singleton();
  if(0==manager) {
    throw cet::exception("PluginManagerNotConfigured")<<"PluginManager::get() was called before PluginManager::configure.";
  }
  return manager;
}

PluginManager&
PluginManager::configure(const Config& iConfig )
{
  PluginManager*& s = singleton();
  if( 0 != s ){
    throw cet::exception("PluginManagerReconfigured");
  }

  Config realConfig = iConfig;
  if (realConfig.searchPath().empty() ) {
    throw cet::exception("PluginManagerEmptySearchPath");
  }
  s = new PluginManager (realConfig);
  return *s;
}


const std::string&
PluginManager::staticallyLinkedLoadingFileName()
{
  static std::string s_name("static");
  return s_name;
}

std::string&
PluginManager::loadingLibraryNamed_()
{
  static std::string s_name(staticallyLinkedLoadingFileName());
  return s_name;
}

PluginManager*& PluginManager::singleton()
{
  static PluginManager* s_singleton;
  return s_singleton;
}

bool
PluginManager::isAvailable()
{
  return 0 != singleton();
}

}
