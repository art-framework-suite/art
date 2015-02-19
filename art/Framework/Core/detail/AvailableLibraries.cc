#include "art/Framework/Core/detail/AvailableLibraries.h"
#include "boost/filesystem.hpp"
#include "cetlib/LibraryManager.h"
#include "cetlib/search_path.h"
#include "cetlib/shlib_utils.h"

#include <iostream>
#include <regex>

namespace bfs = boost::filesystem;
using namespace std::string_literals;
using cet::LibraryManager;
using art::detail::LibraryInfo;

namespace {

  typedef art::ModuleType (ModuleTypeFunc_t) ();
  typedef std::string     (GetSourceLoc_t  ) ();

  std::string getFilePath( LibraryManager const& lm,
                           std::string const& spec,
                           std::string const& pluginType )
  {
    std::string source = "[ not found ]";
    try{
      GetSourceLoc_t* symbolLoc  = nullptr;
      lm.getSymbolByLibspec(spec, "get_source_location",symbolLoc );
      source = (*symbolLoc)();
      bfs::path const p ( source );
      if ( !bfs::exists(p) ) {
        std::regex const format( "/.*/(\\w*_"s+pluginType+".cc)"s );
        source = std::regex_replace(source, format, "/ [ external source ] /$1");
      }
    }
    catch( cet::exception const & e ){
      std::cout << "In: " << __func__ << std::endl;
      std::cout << e.what() << std::endl;
    }
    return source;
  }

  std::string getModuleType( LibraryManager const& lm,
                             std::string const& fullSpec )
  {
    std::string type = "[ error ]";
    try {
      ModuleTypeFunc_t * symbolType = nullptr;
      lm.getSymbolByLibspec(fullSpec, "moduleType", symbolType);
      type = art::to_string( (*symbolType)() );
    }
    catch ( cet::exception const & e ) {
      std::cout << "In: " << __func__ << std::endl;
      std::cout << e.what() << std::endl;
    }
    return type;
  }

  std::string getServiceType( std::string const& fullSpec )

  {
    std::regex  const typeRegex( R"(\S*/(\w*)/\w*)" );
    std::string const tmp_type  = std::regex_replace( fullSpec, typeRegex, "$1" );

    std::string type;
    if      ( tmp_type == "System"   ) type = "system";
    else if ( tmp_type == "Optional" ) type = "optional";
    else type = "user";

    return type;
  }

}

void
art::detail::getModuleLibraries( LibraryInfoContainer& map,
                                 std::string const & pattern)
{
  LibraryManager lm("module",pattern);

  std::vector<std::string> liblist;
  lm.getLoadableLibraries( liblist );

  for ( const auto & lib : liblist ) {

    auto const libspecs = lm.getSpecsByPath( lib );

    std::string const type   = getModuleType( lm, libspecs.second );
    std::string const source = getFilePath( lm, libspecs.second, "module" );
    map.push_back( libspecs.first, LibraryInfo(libspecs.second,lib,source,type) );

  }

}

void
art::detail::getSourceLibraries( LibraryInfoContainer& map,
                                 std::string const & pattern)
{
  LibraryManager lm("source",pattern);

  std::vector<std::string> liblist;
  lm.getLoadableLibraries( liblist );

  for ( const auto & lib : liblist ) {

    auto const libspecs = lm.getSpecsByPath( lib );
    std::string const source = getFilePath( lm, libspecs.second, "source" );
    map.push_back( libspecs.first, LibraryInfo(libspecs.second,lib,source,"source") );

  }

}


void
art::detail::getServiceLibraries( LibraryInfoContainer& map,
                                  std::string const & pattern )
{
  LibraryManager lm( "service", pattern);

  std::vector<std::string> liblist;
  lm.getLoadableLibraries( liblist );

  for ( const auto & lib : liblist ) {

    auto const libspecs = lm.getSpecsByPath( lib );

    std::string const fullSpec = libspecs.second;
    std::string const type     = getServiceType( fullSpec );
    std::string const source   = getFilePath( lm, fullSpec, "service" );

    map.push_back( libspecs.first, LibraryInfo("[ none ]",lib,source,type) );

  }

  // Add by some libraries by hand
  map.push_back( "message", LibraryInfo("[ none ]","[ none ]",
                                        "[ See https://cdcvs.fnal.gov/redmine/projects/art/wiki/Messagefacility ]",
                                        "system"
                                        ) );
}
