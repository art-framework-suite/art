#include "art/Framework/Art/detail/MetadataRegexHelpers.h"

#include <regex>

using namespace std::string_literals;

namespace {
  std::regex const r_module_type  { R"((module_type\s*:\s*)(<string>))" };
  std::regex const r_module_label { R"((.*)(<\d+>)(.*))" };
}

void 
art::detail::replace_module_type( std::string & str, std::string const& spec) {
  str = std::regex_replace( str, r_module_type, "$1"s + spec );
}

void 
art::detail::replace_label( std::string const & label, std::string & str ) {
  std::string const tmp ( str );
  std::size_t const pos = tmp.find_first_of(":");
  if ( pos != std::string::npos && std::regex_search( tmp.begin(), tmp.begin()+pos, r_module_label ) )
    str = std::regex_replace( str, r_module_label, "$1"s + label + "$3"s );
}
