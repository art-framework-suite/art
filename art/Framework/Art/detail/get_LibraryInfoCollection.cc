#include "art/Framework/Art/detail/get_LibraryInfoCollection.h"
#include "art/Framework/Art/detail/PluginSymbolResolvers.h"
#include "art/Framework/EventProcessor/Scheduler.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/LibraryManager.h"
#include "messagefacility/MessageLogger/MFConfig.h"

#include <iomanip>
#include <iostream>
#include <regex>
#include <set>
#include <utility>

using namespace art::detail;
using namespace std::string_literals;
using cet::LibraryManager;

namespace {

  std::string const regex_prefix{"([-A-Za-z0-9]*_)*"};
  std::regex const slash{"/"};
  std::regex const artPrefix{R"(\S*art/.*)"};

  inline std::string
  plugin_suffix(std::size_t const sz)
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
      : libType_{libType}, w_{width}, d_{denom}, v_{verbose}
    {
      if (v_)
        std::cerr << '\n';
    }

    ~StatusBar()
    {
      if (v_)
        std::cerr << '\n';
    }

    void
    print_progress(std::size_t const num) const
    {
      if (!v_)
        return;
      std::size_t const per{100 * num / d_};
      std::cerr << "   Loaded " << std::setw(w_) << std::right << num << '/'
                << d_ << " " << libType_ << plugin_suffix(d_) << " (" << per
                << "%)   " << std::flush;
      std::cerr << '\r';
    }

  private:
    std::string const libType_;
    std::size_t const w_;
    std::size_t const d_;
    bool const v_;
  };

  std::string
  pattern(std::string const& spec)
  {
    std::string const canonSpec = std::regex_replace(spec, slash, "_");
    return regex_prefix + canonSpec + "_";
  }

  inline std::string
  getProvider(std::string const& fullSpec)
  {
    return std::regex_search(fullSpec, artPrefix) ? "art" : "user";
  }

  inline std::vector<std::string>
  getLibraries(LibraryManager const& lm)
  {
    std::vector<std::string> result;
    lm.getLoadableLibraries(result);
    return result;
  }

  bool
  scheduler_included(std::string const& spec, LibraryInfoCollection& result)
  {
    bool const print_only_message = (spec == "scheduler");
    bool const print_available_services = (spec == dflt_spec_pattern());

    if (print_only_message || print_available_services) {
      result.emplace(
        "[ none ]",
        std::make_pair("scheduler", ""),
        "[ none ]",
        std::make_unique<fhicl::WrappedTable<art::Scheduler::Config>>(
          fhicl::Name{"scheduler"}),
        "art",
        "");
      return true;
    }
    return false;
  }

  bool
  messagefacility_included(std::string const& spec,
                           LibraryInfoCollection& result)
  {
    bool const print_only_message = (spec == "message");
    bool const print_available_services = (spec == dflt_spec_pattern());

    if (print_only_message || print_available_services) {
      result.emplace(
        "[ none ]",
        std::make_pair("message", ""),
        "[ See "
        "https://cdcvs.fnal.gov/redmine/projects/art/wiki/Messagefacility ]",
        std::make_unique<fhicl::WrappedTable<mf::MFConfig::Config>>(
          fhicl::Name{"message"}),
        "art",
        "");
      return true;
    }
    return false;
  }

  using suffix_type = art::suffix_type;
  using Suffixes = art::Suffixes;

  std::string
  fhicl_name(std::string const& suffix)
  {
    if (suffix == art::Suffixes::module()) {
      return "<module_label>";
    }
    if (suffix == art::Suffixes::plugin()) {
      return "<plugin_label>";
    }
    if (suffix == art::Suffixes::tool()) {
      return "<tool_label>";
    }
    if (suffix == art::Suffixes::source()) {
      return "source";
    }
    if (suffix == art::Suffixes::mfPlugin()) {
      return "<destination_label>";
    }
    if (suffix == art::Suffixes::mfStatsPlugin()) {
      return "<statistics_destination_label>";
    }
    return "<name>";
  }

  LibraryInfoCollection
  collection_for_plugins(std::string const& suffix,
                         std::string const& spec,
                         bool const verbose)
  {
    LibraryInfoCollection result;
    LibraryManager const lm{suffix, pattern(spec)};
    std::size_t i{};
    auto const& libs = getLibraries(lm);
    auto const sz = libs.size();
    auto const w = std::to_string(sz).size();
    StatusBar const status_bar{lm.libType(), w, sz, verbose};
    for (auto const& lib : libs) {
      auto const& libspecs = lm.getSpecsByPath(lib);
      std::string const& spec =
        libspecs.second.empty() ? libspecs.first : libspecs.second;

      result.emplace(lib,
                     libspecs,
                     getFilePath(lm, spec),
                     getAllowedConfiguration(lm, spec, fhicl_name(suffix)),
                     getProvider(spec),
                     getType(lm, spec));

      status_bar.print_progress(++i);
    }
    return result;
  }

  LibraryInfoCollection
  collection_for_services(std::string const& spec, bool const verbose)
  {
    // These services are not configurable by users.
    std::set<std::string> const systemServicesToIgnore{"TriggerNamesService"};

    LibraryManager const lm{Suffixes::service(), pattern(spec)};
    auto libs = getLibraries(lm);

    // Remove libraries that should be ignored
    libs.erase(std::remove_if(libs.begin(),
                              libs.end(),
                              [&lm, &systemServicesToIgnore](auto const& path) {
                                return cet::search_all(
                                  systemServicesToIgnore,
                                  lm.getSpecsByPath(path).first);
                              }),
               libs.cend());

    auto const sz =
      libs.size() + static_cast<std::size_t>(spec == dflt_spec_pattern());
    StatusBar const status_bar{
      lm.libType(), std::to_string(sz).size(), sz, verbose};
    LibraryInfoCollection result;
    std::size_t i{};
    for (auto const& lib : libs) {
      auto const& libspecs = lm.getSpecsByPath(lib);
      auto const& [shortspec, fullspec] = libspecs;

      result.emplace(lib,
                     libspecs,
                     getFilePath(lm, shortspec), // full specs may be empty
                     getAllowedConfiguration(
                       lm, shortspec, shortspec), // for user-defined servicxes
                     getProvider(fullspec),
                     getType(lm, fullspec));

      status_bar.print_progress(++i);
    }
    if (scheduler_included(spec, result)) {
      status_bar.print_progress(++i);
    }
    if (messagefacility_included(spec, result)) {
      status_bar.print_progress(++i);
    }

    return result;
  }

} // namespace

LibraryInfoCollection
art::detail::get_LibraryInfoCollection(std::string const& suffix,
                                       std::string const& pattern,
                                       bool const verbose)
{
  if (suffix == art::Suffixes::service()) {
    return collection_for_services(pattern, verbose);
  }
  return collection_for_plugins(suffix, pattern, verbose);
}
