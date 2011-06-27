/*
 *  friendlyName.cpp
 *  CMSSW
 */

#include "boost/thread/tss.hpp"
#include "cpp0x/regex"
#include <iostream>
#include <map>
#include <string>

//NOTE:  This should probably be rewritten so that we break the class name into a tree where the template arguments are the node.  On the way down the tree
// we look for '<' or ',' and on the way up (caused by finding a '>') we can apply the transformation to the output string based on the class name for the
// templated class.  Up front we'd register a class name to a transformation function (which would probably take a std::vector<std::string> which holds
// the results of the node transformations)

namespace art {
  namespace friendlyname {
    static std::regex const reBeginSpace("^ +");
    static std::regex const reEndSpace(" +$");
    static std::regex const reAllSpaces(" +");
    static std::regex const reColons("::");
    static std::regex const reComma(",");
    static std::regex const reTemplateArgs("[^<]*<(.*)>$");
    static std::regex const reTemplateClass("([^<>,]+<[^<>]*>)");
    static std::string const emptyString("");

    std::string removeExtraSpaces(std::string const& iIn) {
      return std::regex_replace(std::regex_replace(iIn,reBeginSpace,emptyString),
                                reEndSpace, emptyString);
    }

    std::string removeAllSpaces(std::string const& iIn) {
      return std::regex_replace(iIn, reAllSpaces,emptyString);
    }

    static std::regex const reWrapper("art::Wrapper<(.*)>");
    static std::regex const reString("std::basic_string<char>");
    static std::regex const reUnsigned("unsigned ");
    static std::regex const reLong("long ");
    static std::regex const reVector("std::vector");

    static std::regex const reMapVectorKey("cet::map_vector_key");
    static std::regex const reMapVector("cet::map_vector");

    std::string standardRenames(std::string const& iIn) {
      using std::regex_replace;
      using std::regex;
      std::string name = regex_replace(iIn, reWrapper, "$1");
      name = regex_replace(name,reString,"String");
      name = regex_replace(name,reUnsigned,"u");
      name = regex_replace(name,reLong,"l");
      name = regex_replace(name,reVector,"s");
      name = regex_replace(name,reMapVectorKey,"mvk");
      name = regex_replace(name,reMapVector,"mv");
      return name;
    }

    std::string handleTemplateArguments(std::string const&);
    std::string subFriendlyName(std::string const& iFullName) {
      using namespace boost;
      std::string result = removeExtraSpaces(iFullName);

      smatch theMatch;
      if(regex_match(result,theMatch,reTemplateArgs)) {
        std::string aMatch = theMatch.str(1);
        std::string theSub = handleTemplateArguments(aMatch);
        regex const eMatch(std::string("(^[^<]*)<")+aMatch+">");
        result = regex_replace(result,eMatch,theSub+"$1");
      }
      return result;
    }

    std::string handleTemplateArguments(std::string const& iIn) {
      using namespace boost;
      std::string result = removeExtraSpaces(iIn);
      bool shouldStop = false;
      while(!shouldStop) {
        if(std::string::npos != result.find_first_of("<")) {
          smatch theMatch;
          if(regex_search(result,theMatch,reTemplateClass)) {
            std::string templateClass = theMatch.str(1);
            std::string friendlierName = removeAllSpaces(subFriendlyName(templateClass));
            result = regex_replace(result, regex(templateClass),friendlierName);
          } else {
            std::cout <<" no template match for \""<<result<<"\""<<std::endl;
            assert(0 =="failed to find a match for template class");
          }
        } else {
          shouldStop=true;
        }
      }
      result = regex_replace(result,reComma,"");
      return result;
    }

    std::string friendlyName(std::string const& iFullName) {
      typedef std::map<std::string, std::string> Map;
      static boost::thread_specific_ptr<Map> s_fillToFriendlyName;
      if(0 == s_fillToFriendlyName.get()){
        s_fillToFriendlyName.reset(new Map);
      }
      Map::const_iterator itFound = s_fillToFriendlyName->find(iFullName);
      if(s_fillToFriendlyName->end()==itFound) {
        itFound = s_fillToFriendlyName->insert(Map::value_type(iFullName, subFriendlyName(standardRenames(iFullName)))).first;
      }
      return itFound->second;
    }
  }
}  // art

