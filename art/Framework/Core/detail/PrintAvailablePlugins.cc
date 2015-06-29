#include "art/Framework/Core/detail/AvailableLibraries.h"
#include "art/Framework/Core/detail/PrintAvailablePlugins.h"
#include "art/Framework/Core/detail/PrintFormatting.h"
#include "art/Framework/Core/detail/ServiceNames.h"
#include "cetlib/container_algorithms.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <map>
#include <regex>
#include <set>
#include <string>

using namespace art::detail;
using namespace std::string_literals;
using std::cout;
using std::endl;
using std::string;

namespace {

  //==========================================================================
  // determine column width

  enum { SPEC, TYPE, SRC };

  template <unsigned CLASS = SPEC>
  std::size_t columnWidth( LibraryInfoContainer const& map ) {
    std::size_t s(0);
    cet::for_all( map,[&s](const auto& pr){s = std::max(s,pr.first.size());} );
    return s;
  }

  template <> std::size_t columnWidth<TYPE> ( LibraryInfoContainer const& map ){
    std::size_t s(0);
    cet::for_all( map,[&s](const auto& pr){s = std::max(s,pr.second.type.size());} );
    return s;
  }

  template <> std::size_t columnWidth<SRC> ( LibraryInfoContainer const& map ){
    std::size_t s(0);
    cet::for_all( map,[&s](const auto& pr){s = std::max(s,pr.second.source.size());} );
    return s;
  }

  std::regex const r_module_type( R"((module_type\s*:\s*)(<string>))" );
  std::regex const r_module_label( R"((.*)(<\d+>)(.*))" );

  void replace_module_type( std::string & str, std::string const& spec) {
    str = std::regex_replace( str, r_module_type, "$1"s + spec );
  }

  void replace_label( std::string const & label, std::string & str ) {
    std::string const tmp ( str );
    std::size_t const pos = tmp.find_first_of(":");
    if ( pos != std::string::npos && std::regex_search( tmp.begin(), tmp.begin()+pos, r_module_label ) )
      str = std::regex_replace( str, r_module_label, "$1"s + label + "$3"s );
  }

  //=========================================================================
  std::regex const slash("/");

  void
  adjustPrintedNames(LibraryInfoContainer & libmap)
  {
    static art::detail::ServiceNames const serviceNames;
    for ( auto const& pr : serviceNames ) {
      auto it = libmap.find( pr.first );
      if ( libmap.find( pr.first ) != libmap.cend() ) {
        libmap.emplace( pr.second, it->second );
        libmap.erase( pr.first );
      }
    }
  }

}

void
art::detail::print_available_modules()
{
  LibraryInfoContainer libmap;
  getModuleLibraries(libmap);
  getSourceLibraries(libmap);

  std::size_t const mwidth = columnWidth<SPEC>(libmap);
  std::size_t const swidth = columnWidth<SRC >(libmap);
  std::size_t const twidth = columnWidth<TYPE>(libmap);

  // Print table
  cout << string(indent0().size()+mwidth+4+twidth+4+swidth,'=') << endl << endl;
  cout << indent0()
       << std::setw(mwidth+4) << std::left << "module_type"
       << std::setw(twidth+4) << std::left << "Type"
       << std::left << "Source location"
       << endl;
  cout << string(indent0().size()+mwidth+4+twidth+4+swidth,'-') << endl;
  for ( const auto & pair : libmap ) {
    string const dupl = libmap.count( pair.first ) != 1 ? string(3,'*') : string(3,' ');
    cout << dupl
         << std::setw(mwidth+4) << std::left << pair.first
         << std::setw(twidth+4) << std::left << pair.second.type
         << std::left << pair.second.source
         << endl;
  }
  cout << endl;
  cout << string(3+mwidth+4+twidth+4+swidth,'=') << endl;
}

void
art::detail::print_available_services()
{
  LibraryInfoContainer libmap;
  getServiceLibraries(libmap);
  adjustPrintedNames(libmap);

  std::size_t const mwidth = columnWidth<SPEC>(libmap);
  std::size_t const twidth = columnWidth<TYPE>(libmap);
  std::size_t const swidth = columnWidth<SRC >(libmap);

  // Get service types
  std::map<unsigned,string> types;
  for ( auto & pair : libmap ) {

    const std::string & type = pair.second.type;

    if ( type == "system" ) {
      types.emplace( 0, "system    =  System services automatically loaded by art" );
    }
    else if ( type == "optional" ) {
      types.emplace( 1, "optional  =  Optional services provided by art" );
    }
    else {
      types.emplace( 2, "user      =  Service provided by user/experiment" );
    }

  }

  // Print table
  cout << endl << string(indent0().size()+mwidth+4+twidth+4+swidth,'=') << endl << endl;
  cout << indent0() << "The following types of services are included in the printout below:" << endl << endl;
  for ( const auto & entry : types ) {
    cout << indent__2() << entry.second << endl;
  }

  cout << endl;
  cout << indent0()
       << std::setw(mwidth+4) << std::left << "Services"
       << std::setw(twidth+4) << std::left << "Type"
       << std::left << "Source location"
       << endl;
  cout << string(indent0().size()+mwidth+4+twidth+4+swidth,'=') << endl;
  for ( const auto & pair : libmap ) {
    cout << indent0()
         << std::setw(mwidth+4) << std::left << pair.first
         << std::setw(twidth+4) << std::left << pair.second.type
         << std::left << pair.second.source
         << endl;
  }
  cout << endl;
  cout << string(indent0().size()+mwidth+4+twidth+4+swidth,'=') << endl << endl;

}

void
art::detail::print_module_description( std::vector<std::string> const& mods )
{

  for ( const auto & mod : mods ) {

    std::string const canonSpec = std::regex_replace(mod,slash,"_");
    std::string const pattern = "([-A-Za-z0-9]*_)*"s+canonSpec+"_"s;

    LibraryInfoContainer libmap;
    getModuleLibraries(libmap, pattern, indent__2());
    getSourceLibraries(libmap, pattern);

    std::cout << std::endl << std::string(100,'=') << std::endl << std::endl;

    if ( libmap.empty() ) {
      std::cout << " Module: " << font_bold( mod ) << " not found. " << std::endl;
      continue;
    }

    for ( const auto& libinfo_pair : libmap ) {

      std::cout << indent_1()  << "module_type: " << font_bold( libinfo_pair.first ) << " (or \"" << libinfo_pair.second.altname << "\")" << std::endl << std::endl;
      std::cout << indent__2() << "type   : " << libinfo_pair.second.type << std::endl;
      std::cout << indent__2() << "source : " << libinfo_pair.second.source  << std::endl;
      std::cout << indent__2() << "library: " << libinfo_pair.second.library << std::endl << std::endl;
      std::cout << indent_1()  << "Allowed configuration" << std::endl;
      std::cout << indent_1()  << "---------------------" << std::endl;

      std::string printedConfig = libinfo_pair.second.description;
      replace_module_type ( printedConfig, libinfo_pair.first );
      replace_label( "module_label", printedConfig );

      std::cout << printedConfig << std::endl;
    }

  }

  std::cout << std::endl << std::string(100,'=') << std::endl << std::endl;

}

void
art::detail::print_service_description( std::vector<std::string> const& svcs )
{

  for ( const auto & service : svcs ) {

    static ServiceNames const serviceNames;
    std::string const libbase = serviceNames.libname( service );
    std::string const canonSpec = std::regex_replace(libbase,slash,"_");
    std::string const pattern = "([-A-Za-z0-9]*_)*"s+canonSpec+"_"s;

    LibraryInfoContainer libmap;
    getServiceLibraries(libmap, pattern, indent__2() );

    std::cout << std::endl << std::string(100,'=') << std::endl << std::endl;

    if ( !libmap.count(libbase) ) {
      std::cout << " Service: " << font_bold( service ) << " not found. " << std::endl;
      continue;
    }

    auto iter = libmap.equal_range( libbase ).first;
    // Handle cases where (e.g.) 'FloatingPointControl' is specified
    // when 'floating_point_control' should be.
    std::string const printed_name = serviceNames.fclname( service );

    std::cout << indent_1()  << "service    : " << font_bold( printed_name ) << std::endl << std::endl;
    std::cout << indent__2() << "type   : " << iter->second.type << std::endl;
    std::cout << indent__2() << "source : " << iter->second.source  << std::endl;
    std::cout << indent__2() << "library: " << iter->second.library << std::endl << std::endl;
    std::cout << indent_1()  << "Allowed configuration" << std::endl;
    std::cout << indent_1()  << "---------------------" << std::endl;

    std::string printedConfig = iter->second.description;
    replace_label( iter->first, printedConfig );

    std::cout << printedConfig << std::endl;
  }

  std::cout << std::endl << std::string(100,'=') << std::endl << std::endl;
}
