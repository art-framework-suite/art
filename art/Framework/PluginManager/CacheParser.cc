// ======================================================================
//
// Package:     PluginManager
// Class  :     CacheParser
//
// ======================================================================


#include "art/Framework/PluginManager/CacheParser.h"
#include "art/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include <algorithm>
#include <limits>


using namespace cet;
using namespace std;


namespace artplugin {

//
// static member functions
//
  static void checkForError(const istream& iIn,
                            unsigned long iRecordNumber,
                            const string& iContext)
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
CacheParser::readline(istream& iIn, const boost::filesystem::path& iDirectory,
         unsigned long iRecordNumber, PluginInfo &oInfo, string& oPluginType)
{
  static const string kNewLine("start of new line");
  string fileName;
  string pluginName;
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
  iIn.ignore(numeric_limits<int>::max(),
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
CacheParser::read(istream& iIn,
                  const boost::filesystem::path& iDirectory,
                  CacheParser::CategoryToInfos& iOut)
{
#if 0
  unsigned long recordNumber=0;

  string pluginType;

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
    stable_sort(it->second.begin(),it->second.end(), CompPluginInfos());
  }
#endif
}

void
CacheParser::write(const CategoryToInfos& iInfos, ostream& oOut)
{
#if 0
  //order the data more to our liking: library then object then type
  LoadableToPlugins ordered;

  for(CategoryToInfos::const_iterator it = iInfos.begin();
      it != iInfos.end();
      ++it) {
    string type(it->first);
    for(vector<PluginInfo>::const_iterator it2=it->second.begin();
        it2 != it->second.end();
        ++it2) {
      //remove any directory specification
      string loadable(it2->loadable_.leaf());
      string name(it2->name_);
      ordered[loadable].push_back(NameAndType(name,type));
    }
  }
  write(ordered,oOut);
#endif
}

void
CacheParser::write(LoadableToPlugins& iIn, ostream& oOut)
{
#if 0
  for( LoadableToPlugins::iterator it = iIn.begin();
       it!=iIn.end();
       ++it) {
    string loadable(it->first.string());
    replaceSpaces(loadable);
    art::sort_all(it->second);

    for(vector<pair<string,string> >::iterator it2 = it->second.begin();
        it2 != it->second.end();
        ++it2) {
      oOut << loadable <<" "<<replaceSpaces(it2->first)<<" "<<replaceSpaces(it2->second)<<"\n";
    }
  }
#endif
}

void
CacheParser::read(istream& iIn, LoadableToPlugins& oOut)
{
#if 0
  unsigned long recordNumber=0;

  string pluginType;

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

string&
CacheParser::replaceSpaces(string& io)
{
  string::size_type index=0;
  while(string::npos != (index = io.find_first_of(" \t\n",index))) {
    io[index]='%';
  }
  return io;
}

string& CacheParser::restoreSpaces(string& io)
{
  string::size_type index=0;
  while(string::npos != (index = io.find_first_of("%",index))) {
    io[index]=' ';
  }
  return io;
}

}  // namespace artplugin
