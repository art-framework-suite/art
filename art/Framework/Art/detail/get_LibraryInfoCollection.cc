#include "art/Framework/Art/detail/PluginSymbolResolvers.h"
#include "art/Framework/Art/detail/ServiceNames.h"
#include "art/Framework/Art/detail/get_LibraryInfoCollection.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/LibraryManager.h"

#include <regex>
#include <set>
#include <utility>

using namespace art::detail;
using namespace std::string_literals;
using cet::LibraryManager;

namespace {

  std::string const empty_description {"\n" + indent__2() + "[ provided in future release ]\n"};
  std::string const regex_prefix {"([-A-Za-z0-9]*_)*"};
  std::regex  const slash {"/"};

  std::string pattern(std::string const& spec)
  {
    std::string const canonSpec = std::regex_replace(spec, slash, "_");
    return regex_prefix + canonSpec + "_";
  }

  inline std::string getProvider(std::string const& fullSpec)
  {
    return std::regex_search(fullSpec, std::regex{ R"(\S*art/.*)" } ) ? "art" : "user";
  }

  inline std::vector<std::string> getLibraries(LibraryManager const& lm)
  {
    std::vector<std::string> result;
    lm.getLoadableLibraries(result);
    return result;
  }

  void maybe_include_messagefacility(std::string const& spec,
                                     LibraryInfoCollection& result)
  {
    bool const print_only_message       = (spec == "message");
    bool const print_available_services = (spec == dflt_spec_pattern());

    if ( print_only_message || print_available_services ) {
      result.emplace( "[ none ]",
                      std::make_pair("message",""),
                      "[ See https://cdcvs.fnal.gov/redmine/projects/art/wiki/Messagefacility ]",
                      empty_description,
                      "art",
                      "" );
    }
  }

  using suffix_type = art::suffix_type;
  using Suffixes = art::Suffixes;

  std::string fhicl_name(suffix_type const st)
  {
    switch (st) {
    case suffix_type::module: return "<module_label>";
    case suffix_type::plugin: return "<plugin_label>";
    case suffix_type::source: return "source";
    default :
      throw art::Exception(art::errors::LogicError)
        << "The " << Suffixes::get(st) << "is not supported for function: " << __func__ << '\n';
    }
  }

  template <suffix_type st>
  LibraryInfoCollection getCollection(std::string const& spec,
                                      std::string const& tab)
  {
    LibraryInfoCollection result;
    LibraryManager const lm {Suffixes::get(st), pattern(spec)};
    for (auto const& lib : getLibraries(lm)) {

      auto const& libspecs = lm.getSpecsByPath(lib);
      std::string const& spec = libspecs.second.empty() ? libspecs.first : libspecs.second;

      std::string const& type     = getType<st>(lm, spec);
      std::string const& path     = getFilePath<st>(lm, spec);
      std::string const& provider = getProvider(spec);
      std::string const& desc     = getDescription<st>(lm, spec, fhicl_name(st), tab);

      result.emplace(lib, libspecs, path, desc, provider, type);
    }

    return result;
  }

  template <>
  LibraryInfoCollection
  getCollection<suffix_type::service>(std::string const& spec,
                                      std::string const& tab)
  {
    // These services are not configurable by users.
    std::set<std::string> const systemServicesToIgnore {
      "CurrentModule",
      "PathSelection",
      "ScheduleContext",
      "TriggerNamesService"
    };

    std::string const& pSpec = ServiceNames::libname(spec);

    LibraryInfoCollection result;
    LibraryManager const lm {Suffixes::get(suffix_type::service), pattern(pSpec)};
    for ( auto const& lib : getLibraries(lm) ) {

      auto const& libspecs = lm.getSpecsByPath(lib);

      // Skip non-configurable system services
      if (systemServicesToIgnore.find(libspecs.first) != systemServicesToIgnore.cend()) continue;

      std::string const& spec     = libspecs.first;
      std::string const& fullspec = libspecs.second;
      std::string const& type     = getType<suffix_type::service>( lm, libspecs.second );
      std::string const& provider = getProvider(fullspec);
      std::string const& path     = getFilePath<suffix_type::service>(lm, spec); // full specs may be empty

      auto const& fclname = ServiceNames::fclname(spec);
      std::string const& desc = getDescription<suffix_type::service>(lm, spec, fclname, tab); // for user-defined servicxes

      result.emplace(lib, std::make_pair(fclname, fullspec), path, desc, provider, type);
    }

    maybe_include_messagefacility(spec, result);

    return result;
  }


} // namespace

LibraryInfoCollection
art::detail::get_LibraryInfoCollection(suffix_type const st,
                                       std::string const& pattern,
                                       std::string const& tab)
{
  switch(st) {
  case suffix_type::module  : return getCollection<suffix_type::module >(pattern, tab);
  case suffix_type::service : return getCollection<suffix_type::service>(pattern, tab);
  case suffix_type::source  : return getCollection<suffix_type::source >(pattern, tab);
  default                   : return getCollection<suffix_type::plugin >(pattern, tab);
  }
}
