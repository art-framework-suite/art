#include "art/Framework/Art/detail/PluginSymbolResolvers.h"
#include "art/Framework/Art/detail/ServiceNames.h"
#include "art/Framework/Art/detail/get_LibraryInfoCollection.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/LibraryManager.h"

#include <iomanip>
#include <iostream>
#include <regex>
#include <set>
#include <utility>

using namespace art::detail;
using namespace std::string_literals;
using cet::LibraryManager;

namespace {

  std::string const regex_prefix {"([-A-Za-z0-9]*_)*"};
  std::regex  const slash {"/"};

  inline std::string plugin_suffix(std::size_t const sz)
  {
    return sz != 1ull ? "s" : "";
  }

  // Auxiliary class for creating status bar.
  class StatusBar {
  public:

    StatusBar(std::string const& libType,
              std::size_t const width,
              std::size_t const denom,
              bool const verbose)
      : libType_{libType}
      , w_{width}
      , d_{denom}
      , v_{verbose}
    {
      if (v_) std::cerr << '\n';
    }

    ~StatusBar(){
      if (v_) std::cerr << '\n';
    }

    void print_progress(std::size_t const num) const
    {
      if (!v_)
        return;
      std::size_t const per {100*num/d_};
      std::cerr << "   Loaded "
                << std::setw(w_) << std::right << num << '/' << d_
                << " " << libType_ << plugin_suffix(d_)
                << " (" << per << "%)   " << std::flush;
      std::cerr << '\r';
    }
  private:
    std::string const& libType_;
    std::size_t const w_;
    std::size_t const d_;
    bool const v_;
  };

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

  bool messagefacility_included(std::string const& spec,
                                LibraryInfoCollection& result)
  {
    bool const print_only_message = (spec == "message");
    bool const print_available_services = (spec == dflt_spec_pattern());

    if (print_only_message || print_available_services) {
      result.emplace( "[ none ]",
                      std::make_pair("message",""),
                      "[ See https://cdcvs.fnal.gov/redmine/projects/art/wiki/Messagefacility ]",
                      std::unique_ptr<art::ConfigurationTable>{nullptr},
                      "art",
                      "" );
      return true;
    }
    return false;
  }

  using suffix_type = art::suffix_type;
  using Suffixes = art::Suffixes;

  std::string fhicl_name(suffix_type const st)
  {
    switch (st) {
    case suffix_type::module: return "<module_label>";
    case suffix_type::plugin: return "<plugin_label>";
    case suffix_type::tool  : return "<tool_label>";
    case suffix_type::source: return "source";
    default :
      throw art::Exception(art::errors::LogicError)
        << "The '" << Suffixes::get(st) << "' suffix is not supported for function: " << __func__ << '\n';
    }
  }

  template <suffix_type st>
  LibraryInfoCollection getCollection(std::string const& spec,
                                      bool const verbose)
  {
    LibraryInfoCollection result;
    LibraryManager const lm {Suffixes::get(st), pattern(spec)};
    std::size_t i{};
    auto const& libs = getLibraries(lm);
    auto const sz = libs.size();
    auto const w = std::to_string(sz).size();
    StatusBar const status_bar {lm.libType(), w, sz, verbose};
    for (auto const& lib : libs) {
      auto const& libspecs = lm.getSpecsByPath(lib);
      std::string const& spec = libspecs.second.empty() ? libspecs.first : libspecs.second;

      result.emplace(lib,
                     libspecs,
                     getFilePath<st>(lm, spec),
                     getAllowedConfiguration<st>(lm, spec, fhicl_name(st)),
                     getProvider(spec),
                     getType<st>(lm, spec));

      status_bar.print_progress(++i);
    }
    return result;
  }

  template <>
  LibraryInfoCollection
  getCollection<suffix_type::service>(std::string const& spec,
                                      bool const verbose)
  {
    // These services are not configurable by users.
    std::set<std::string> const systemServicesToIgnore {
      "CurrentModule",
      "PathSelection",
      "ScheduleContext",
      "TriggerNamesService"
    };

    std::string const& pSpec = ServiceNames::libname(spec);

    LibraryManager const lm {Suffixes::get(suffix_type::service), pattern(pSpec)};
    auto libs = getLibraries(lm);

    // Remove libraries that should be ignored
    libs.erase(std::remove_if(libs.begin(),
                              libs.end(),
                              [&lm, &systemServicesToIgnore](auto const& path) {
                                return cet::search_all(systemServicesToIgnore, lm.getSpecsByPath(path).first);
                              }),
               libs.cend());

    auto const sz = libs.size() + static_cast<std::size_t>(spec == dflt_spec_pattern());
    StatusBar const status_bar {lm.libType(), std::to_string(sz).size(), sz, verbose};
    LibraryInfoCollection result;
    std::size_t i {};
    for (auto const& lib : libs) {
      auto const& libspecs = lm.getSpecsByPath(lib);
      std::string const& spec = libspecs.first;
      std::string const& fullspec = libspecs.second;
      auto const& fclname = ServiceNames::fclname(spec);

      result.emplace(lib,
                     std::make_pair(fclname, fullspec),
                     getFilePath<suffix_type::service>(lm, spec),                      // full specs may be empty
                     getAllowedConfiguration<suffix_type::service>(lm, spec, fclname), // for user-defined servicxes
                     getProvider(fullspec),
                     getType<suffix_type::service>(lm, libspecs.second));

      status_bar.print_progress(++i);
    }
    if (messagefacility_included(spec, result)) {
      status_bar.print_progress(++i);
    }

    return result;
  }


} // namespace

LibraryInfoCollection
art::detail::get_LibraryInfoCollection(suffix_type const st,
                                       std::string const& pattern,
                                       bool const verbose)
{
  switch(st) {
  case suffix_type::module  : return getCollection<suffix_type::module >(pattern, verbose);
  case suffix_type::service : return getCollection<suffix_type::service>(pattern, verbose);
  case suffix_type::source  : return getCollection<suffix_type::source >(pattern, verbose);
  case suffix_type::plugin  : return getCollection<suffix_type::plugin >(pattern, verbose);
  case suffix_type::tool    : return getCollection<suffix_type::tool   >(pattern, verbose);
    // No default - allow compiler to warn if missing suffix_type.
  }
  return {};
}
