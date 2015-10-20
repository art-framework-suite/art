#include "art/Framework/Core/detail/AvailableLibraries.h"
#include "art/Framework/Core/detail/PrintFormatting.h"
#include "boost/filesystem.hpp"
#include "cetlib/LibraryManager.h"
#include "cetlib/search_path.h"
#include "cetlib/shlib_utils.h"

#include <iostream>
#include <regex>

namespace bfs = boost::filesystem;
using namespace art::detail;
using namespace std::string_literals;
using cet::LibraryManager;

namespace {

  std::string const empty_description = "\n"s + indent__2() + "[ provided in future release ]"s;

  typedef art::ModuleType (ModuleTypeFunc_t) ();
  typedef std::string     (GetSourceLoc_t  ) ();
  typedef std::ostream&   (GetDescription_t) (std::ostream&, std::string const&);

  std::string getFilePath( LibraryManager const& lm,
                           std::string const& fullspec,
                           std::string const& pluginType )
  {
    std::string source = "[ not found ]";
    try{
      GetSourceLoc_t* symbolLoc  = nullptr;
      lm.getSymbolByLibspec(fullspec, "get_source_location",symbolLoc );
      source = (*symbolLoc)();
      bfs::path const p ( source );
      if ( !bfs::exists(p) ) {
        source = "/ [ external source ] /"s+fullspec+"_"s+pluginType+".cc";
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

  std::string getDescription( LibraryManager const& lm,
                              std::string const& fullSpec,
                              std::string const& tab)
  {
    std::string description;
    try {
      GetDescription_t * symbolType = nullptr;
      lm.getSymbolByLibspec(fullSpec, "print_description", symbolType);
      std::ostringstream oss;
      (*symbolType)(oss,tab);
      description = oss.str();
    }
    catch ( cet::exception const & e ) {
      std::cout << "In: " << __func__ << std::endl;
      std::cout << e.what() << std::endl;
    }
    return description;
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
                                 std::string const & pattern,
                                 std::string const & tab)
{
  LibraryManager lm("module",pattern);

  std::vector<std::string> liblist;
  lm.getLoadableLibraries( liblist );

  for ( const auto & lib : liblist ) {

    auto const & libspecs = lm.getSpecsByPath( lib );

    std::string const type   = getModuleType ( lm, libspecs.second );
    std::string const source = getFilePath   ( lm, libspecs.second, "module" );
    std::string const desc   = getDescription( lm, libspecs.second, tab );
    map.emplace( libspecs.first, LibraryInfo(libspecs.second,lib,source,type,desc) );

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
    map.emplace( libspecs.first, LibraryInfo(libspecs.second,lib,source,"source",
                                             empty_description ) );

  }

}

namespace {

  // These services are not configurable by users.
  std::set<std::string> const systemServicesToIgnore {
    "CurrentModule",
    "PathSelection",
    "ScheduleContext",
    "TriggerNamesService"
  };


}

void
art::detail::getServiceLibraries( LibraryInfoContainer& map,
                                  std::string const & pattern,
                                  std::string const & tab)
{
  LibraryManager lm( "service", pattern);

  std::vector<std::string> liblist;
  lm.getLoadableLibraries( liblist );

  for ( const auto & lib : liblist ) {

    auto const & libspecs = lm.getSpecsByPath( lib );

    // Skip non-configurable system services
    if ( systemServicesToIgnore.find( libspecs.first ) !=
         systemServicesToIgnore.cend() ) continue;

    std::string const spec     = libspecs.first;
    std::string const fullspec = libspecs.second;
    std::string const type     = getServiceType( fullspec );
    std::string const source   = getFilePath( lm, spec, "service" ); // full specs may be empty
    std::string const desc     = getDescription( lm, spec, tab );    // for user-defined services

    map.emplace( spec, LibraryInfo("[ none ]",lib,source,type,desc) );

  }

  // Message facility
  map.emplace( "message", LibraryInfo("[ none ]","[ none ]",
                                      "[ See https://cdcvs.fnal.gov/redmine/projects/art/wiki/Messagefacility ]",
                                      "system",
                                      empty_description
                                      ) );

}
