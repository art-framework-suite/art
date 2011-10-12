#include "art/Utilities/FriendlyName.h"
#include "cpp0x/regex"

#include "boost/thread/tss.hpp"

#include <iostream>
#include <map>
#include <string>

// NOTE: This should probably be rewritten so that we break the class
// name into a tree where the template arguments are the node.  On the
// way down the tree we look for '<' or ',' and on the way up (caused by
// finding a '>') we can apply the transformation to the output string
// based on the class name for the templated class.  Up front we'd
// register a class name to a transformation function (which would
// probably take a std::vector<std::string> which holds the results of
// the node transformations).

namespace {
  static std::regex const reBeginSpace("^ +");
  static std::regex const reEndSpace(" +$");
  static std::string const emptyString("");

  std::string removeExtraSpaces(std::string const & iIn)
  {
    return std::regex_replace(std::regex_replace(iIn, reBeginSpace, emptyString),
                              reEndSpace, emptyString);
  }

  static std::regex const reAllSpaces(" +");
  std::string removeAllSpaces(std::string const & iIn)
  {
    return std::regex_replace(iIn, reAllSpaces, emptyString);
  }

  static std::regex const reWrapper("art::Wrapper<(.*)>");
  static std::regex const reString("std::basic_string<char>");
  static std::regex const reUnsigned("unsigned ");
  static std::regex const reLong("long ");
  static std::regex const reVector("std::vector");
  static std::regex const reMapVectorKey("cet::map_vector_key");
  static std::regex const reMapVector("cet::map_vector");

  std::string standardRenames(std::string const & iIn)
  {
    std::string name = std::regex_replace(iIn, reWrapper, "$1");
    name = std::regex_replace(name, reString, "String");
    name = std::regex_replace(name, reUnsigned, "u");
    name = std::regex_replace(name, reLong, "l");
    name = std::regex_replace(name, reVector, "s");
    name = std::regex_replace(name, reMapVectorKey, "mvk");
    name = std::regex_replace(name, reMapVector, "mv");
    return name;
  }

  static std::regex const reTemplateArgs("([^<]*)<(.*)>$");

  // Declaration required here because handleTemplateArguments and
  // subFriendlyName call each other.
  std::string handleTemplateArguments(std::string const &, std::string const &);
  std::string subFriendlyName(std::string const & iFullName)
  {
    std::string result = removeExtraSpaces(iFullName);
    std::smatch theMatch;
    if (std::regex_match(result, theMatch, reTemplateArgs)) {
      std::string cMatch = theMatch.str(1);
      std::string aMatch = theMatch.str(2);
      std::string theSub = handleTemplateArguments(cMatch, aMatch);
      std::regex const eMatch(std::string("^") + cMatch + '<' + aMatch + '>');
      result = std::regex_replace(result, eMatch, theSub + cMatch);
    }
    return result;
  }

  static std::regex const reFirstTwoArgs("^([^,]+),([^,]+)");

  void maybeSwapFirstTwoArgs(std::string & result)
  {
    std::smatch theMatch;
    if (std::regex_search(result, theMatch, reFirstTwoArgs) &&
        (theMatch.str(1) > theMatch.str(2))) {
      result = std::regex_replace(result, reFirstTwoArgs, "$2,$1");
    }
  }

  static std::regex const reComma(",");
  static std::regex const reTemplateClass("([^<>,]+<[^<>]*>)");
  static std::regex const reAssns("art::Assns");

  std::string handleTemplateArguments(std::string const & cName, std::string const & tArgs)
  {
    std::string result = removeExtraSpaces(tArgs);
    bool shouldStop = false;
    while (!shouldStop) {
      if (std::string::npos != result.find_first_of("<")) {
        std::smatch theMatch;
        if (std::regex_search(result, theMatch, reTemplateClass)) {
          std::string templateClass = theMatch.str(1);
          std::string friendlierName = removeAllSpaces(subFriendlyName(templateClass));
          result = std::regex_replace(result, std::regex(templateClass), friendlierName);
        }
        else {
          std::cout << " no template match for \"" << result << "\"" << std::endl;
          assert(0 == "failed to find a match for template class");
        }
      }
      else {
        shouldStop = true;
      }
    }
    if (std::regex_match(cName, reAssns)) { maybeSwapFirstTwoArgs(result); }
    result = std::regex_replace(result, reComma, "");
    return result;
  }

}

std::string art::friendlyname::friendlyName(std::string const & iFullName)
{
  typedef std::map<std::string, std::string> Map;
  static boost::thread_specific_ptr<Map> s_fillToFriendlyName;
  if (0 == s_fillToFriendlyName.get()) {
    s_fillToFriendlyName.reset(new Map);
  }
  Map::const_iterator itFound = s_fillToFriendlyName->find(iFullName);
  if (s_fillToFriendlyName->end() == itFound) {
    itFound = s_fillToFriendlyName->insert(Map::value_type(iFullName, subFriendlyName(standardRenames(iFullName)))).first;
  }
  return itFound->second;
}

