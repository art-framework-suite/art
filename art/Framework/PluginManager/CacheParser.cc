// -*- C++ -*-
//
// Package:     PluginManager
// Class  :     CacheParser
//
// Implementation:
//     <Notes on implementation>
//
// Original Author:  Chris Jones
//         Created:  Wed Apr  4 14:30:46 EDT 2007
//
//

// system include files
#include <algorithm>
#include <limits>

// user include files
#include "art/Framework/PluginManager/CacheParser.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/Algorithms.h"

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
//CacheParser::CacheParser(std::istream&)
//{
//}

// CacheParser::CacheParser(const CacheParser& rhs)
// {
//    // do actual copying here;
// }

//CacheParser::~CacheParser()
//{
//}

//
// assignment operators
//
// const CacheParser& CacheParser::operator=(const CacheParser& rhs)
// {
//   //An exception safe implementation is
//   CacheParser temp(rhs);
//   swap(rhs);
//
//   return *this;
// }

//
// member functions
//

//
// const member functions
//

//
// static member functions
//
  static void checkForError(const std::istream& iIn,
                            unsigned long iRecordNumber,
                            const std::string& iContext)
{
    if(iIn.eof()) {
      throw artZ::Exception("PluginCacheParseFailed")<<"Unexpectedly reached end of file for line "
      <<iRecordNumber<<" just after '"<<iContext<<"'";
    }
    if(iIn.bad()) {
      throw artZ::Exception("PluginCacheParseFailed")<<"Reading failed on line "<<iRecordNumber <<" just after '"<<iContext<<"'";
    }
}

bool
CacheParser::readline(std::istream& iIn, const boost::filesystem::path& iDirectory,
         unsigned long iRecordNumber, PluginInfo &oInfo, std::string& oPluginType)
{
  static const std::string kNewLine("start of new line");
  std::string fileName;
  std::string pluginName;
  iIn >> fileName;
  if(iIn.eof()) { return false;}
  checkForError(iIn,iRecordNumber,kNewLine);
  CacheParser::restoreSpaces(fileName);
  iIn >> pluginName;
  checkForError(iIn,iRecordNumber,fileName);
  CacheParser::restoreSpaces(pluginName);
  iIn >> oPluginType;
  checkForError(iIn,iRecordNumber,oPluginType);
  CacheParser::restoreSpaces(oPluginType);

  oInfo.loadable_ = iDirectory / fileName;
  oInfo.name_ = pluginName;

  //ignore everything to the end of line
  iIn.ignore(std::numeric_limits<int>::max(),
             '\n');
  while(iIn.peek() == '\n') {
    iIn.get();
  }
  return true;
}

namespace {
  struct CompPluginInfos {
    bool operator()(const PluginInfo& iLHS,
                    const PluginInfo& iRHS) const
  {
    return iLHS.name_ < iRHS.name_;
  }
  };
}

void
CacheParser::read(std::istream& iIn,
                  const boost::filesystem::path& iDirectory,
                  CacheParser::CategoryToInfos& iOut)
{
#if 0
  unsigned long recordNumber=0;

  std::string pluginType;

  PluginInfo info;

  while(iIn) {
    ++recordNumber;
    if( not readline(iIn,iDirectory,recordNumber,info,pluginType) ) {
      break;
    }
    iOut[pluginType].push_back(info);
  }
  //now do a sort which preserves any previous order for files
  for(CacheParser::CategoryToInfos::iterator it = iOut.begin(), itEnd=iOut.end();
      it != itEnd;
      ++it) {
    std::stable_sort(it->second.begin(),it->second.end(), CompPluginInfos());
  }
#endif
}

void
CacheParser::write(const CategoryToInfos& iInfos, std::ostream& oOut)
{
#if 0
  //order the data more to our liking: library then object then type
  LoadableToPlugins ordered;

  for(CategoryToInfos::const_iterator it = iInfos.begin();
      it != iInfos.end();
      ++it) {
    std::string type(it->first);
    for(std::vector<PluginInfo>::const_iterator it2=it->second.begin();
        it2 != it->second.end();
        ++it2) {
      //remove any directory specification
      std::string loadable(it2->loadable_.leaf());
      std::string name(it2->name_);
      ordered[loadable].push_back(NameAndType(name,type));
    }
  }
  write(ordered,oOut);
#endif
}

void
CacheParser::write(LoadableToPlugins& iIn, std::ostream& oOut)
{
#if 0
  for( LoadableToPlugins::iterator it = iIn.begin();
       it!=iIn.end();
       ++it) {
    std::string loadable(it->first.string());
    replaceSpaces(loadable);
    art::sort_all(it->second);

    for(std::vector<std::pair<std::string,std::string> >::iterator it2 = it->second.begin();
        it2 != it->second.end();
        ++it2) {
      oOut << loadable <<" "<<replaceSpaces(it2->first)<<" "<<replaceSpaces(it2->second)<<"\n";
    }
  }
#endif
}

void
CacheParser::read(std::istream& iIn, LoadableToPlugins& oOut)
{
#if 0
  unsigned long recordNumber=0;

  std::string pluginType;

  PluginInfo info;
  NameAndType pat;
  boost::filesystem::path empty;

  while(iIn) {
    ++recordNumber;
    if( not readline(iIn,empty,recordNumber,info,pat.second) ) {
      break;
    }
    pat.first = info.name_;
    oOut[info.loadable_].push_back(pat);
  }
#endif
}

std::string&
CacheParser::replaceSpaces(std::string& io)
{
  std::string::size_type index=0;
  while(std::string::npos != (index = io.find_first_of(" \t\n",index))) {
    io[index]='%';
  }
  return io;
}

std::string& CacheParser::restoreSpaces(std::string& io)
{
  std::string::size_type index=0;
  while(std::string::npos != (index = io.find_first_of("%",index))) {
    io[index]=' ';
  }
  return io;
}
}
