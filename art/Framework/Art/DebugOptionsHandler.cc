#include "art/Framework/Art/DebugOptionsHandler.h"

#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <regex>
#include <string>

using namespace std::string_literals;
using art::detail::fhicl_key;
using table_t = fhicl::extended_value::table_t;

namespace {
  std::pair<std::string, bool>
  destination_via_env()
  {
    char const* debug_config{getenv("ART_DEBUG_CONFIG")};
    if (debug_config == nullptr)
      return std::make_pair("", false);

    try {
      // Check if the provided character string is a file name
      std::string fn;
      if (std::regex_match(debug_config, std::regex("[[:alpha:]/\\.].*"))) {
        fn = debug_config;
      }
      std::cerr << "** ART_DEBUG_CONFIG is defined **\n";
      return std::make_pair(fn, true);
    }
    catch (std::regex_error const& e) {
      std::cerr << "REGEX ERROR: " << e.code() << ".\n";
    }
    return std::make_pair("", false);
  }
}

art::DebugOptionsHandler::DebugOptionsHandler(bpo::options_description& desc,
                                              std::string const& basename)
{
  bpo::options_description debug_options{"Debugging options"};
  auto options = debug_options.add_options();
  add_opt(options,
          "mt-diagnostics,M",
          bpo::value<std::string>(),
          "Log art-specific multi-threading diagnostics to "
          "the provided destination.");
  add_opt(options, "trace", "Activate tracing.");
  add_opt(options, "notrace", "Deactivate tracing.");
  add_opt(
    options, "timing", "Activate monitoring of time spent per event/module.");
  add_opt(options,
          "timing-db",
          bpo::value<std::string>(),
          "Output time-tracking data to SQLite3 database with name <db-file>.");
  add_opt(options, "notiming", "Deactivate time tracking.");
  add_opt(options,
          "memcheck",
          "Activate monitoring of memory use (deprecated--per-job "
          "memory information printed in job summary).");
  add_opt(options,
          "memcheck-db",
          bpo::value<std::string>(),
          "Output memory use data to SQLite3 database with name <db-file>.");
  add_opt(options, "nomemcheck", "Deactivate monitoring of memory use.");
  add_opt(options,
          "data-dependency-graph,g",
          bpo::value<std::string>(),
          "Print DOT file that shows the dependency graph of "
          "modules, based on the specified paths and 'consumes' "
          "statements invoked by users; call constructors of all "
          "modules and exit just before processing the event loop.");
  add_opt(
    options,
    "validate-config",
    bpo::value<std::string>(),
    "Output post-processed configuration to <file>; call constructors of all "
    "sources, modules and services, performing extra configuration "
    "verification.  Exit just before processing the event loop.");
  add_opt(
    options,
    "debug-config",
    bpo::value<std::string>(),
    ("Output post-processed configuration to <file> and exit. Equivalent to env ART_DEBUG_CONFIG=<file> "s +
     basename + " ...")
      .c_str());
  add_opt(
    options,
    "config-out",
    bpo::value<std::string>(),
    "Output post-processed configuration to <file> and continue with job.");
  add_opt(
    options, "annotate", "Include configuration parameter source information.");
  add_opt(options,
          "prefix-annotate",
          "Include configuration parameter source information "
          "on line preceding parameter declaration.");
  desc.add(debug_options);
}

int
art::DebugOptionsHandler::doCheckOptions(bpo::variables_map const& vm)
{
  if (vm.count("trace") + vm.count("notrace") > 1) {
    throw Exception(errors::Configuration)
      << "Options --trace and --notrace are incompatible.\n";
  }
  if (vm.count("timing") + vm.count("notiming") > 1) {
    throw Exception(errors::Configuration)
      << "Options --timing and --notiming are incompatible.\n";
  }
  if (vm.count("timing-db") + vm.count("notiming") > 1) {
    throw Exception(errors::Configuration)
      << "Options --timing-db and --notiming are incompatible.\n";
  }
  if (vm.count("memcheck") + vm.count("nomemcheck") > 1) {
    throw Exception(errors::Configuration)
      << "Options --memcheck and --nomemcheck are incompatible.\n";
  }
  if (vm.count("memcheck-db") + vm.count("nomemcheck") > 1) {
    throw Exception(errors::Configuration)
      << "Options --memcheck-db and --nomemcheck are incompatible.\n";
  }
  if (vm.count("validate-config") + vm.count("debug-config") +
        vm.count("config-out") >
      1) {
    throw Exception(errors::Configuration)
      << "Options --validate-config, --debug-config, and --config-out are "
         "incompatible.\n";
  }
  if (vm.count("annotate") + vm.count("prefix-annotate") > 1) {
    throw Exception(errors::Configuration)
      << "Options --annotate and --prefix-annotate are incompatible.\n";
  }

  if (vm.count("annotate") + vm.count("prefix-annotate") == 1 &&
      vm.count("debug-config") + vm.count("config-out") == 0) {
    throw Exception(errors::Configuration)
      << "Options --annotate and --prefix-annotate must be specified with "
         "either --debug-config or --config-out.\n";
  }

  return 0;
}

int
art::DebugOptionsHandler::doProcessOptions(
  bpo::variables_map const& vm,
  fhicl::intermediate_table& raw_config)
{

  using namespace fhicl::detail;

  auto const scheduler_key = fhicl_key("services", "scheduler");
  std::string debug_table;

  // Get ART_DEBUG_CONFIG value
  std::string fn;
  auto const result = destination_via_env();
  if (result.second) {
    debug_table = fhicl_key(scheduler_key, "debugConfig");
    fn = result.first;
  }

  // "validate-config" and "debug-config" win over ART_DEBUG_CONFIG
  if (vm.count("validate-config")) {
    debug_table = fhicl_key(scheduler_key, "validateConfig");
    fn = vm["validate-config"].as<std::string>();
  } else if (vm.count("debug-config")) {
    debug_table = fhicl_key(scheduler_key, "debugConfig");
    fn = vm["debug-config"].as<std::string>();
  } else if (vm.count("config-out")) {
    debug_table = fhicl_key(scheduler_key, "configOut");
    fn = vm["config-out"].as<std::string>();
  }
  if (!debug_table.empty()) {
    raw_config.put(fhicl_key(debug_table, "fileName"), fn);
  }

  std::string mode{"raw"};
  if (vm.count("annotate")) {
    mode = "annotate";
  }
  if (vm.count("prefix-annotate")) {
    mode = "prefix-annotate";
  }
  if (!debug_table.empty()) {
    raw_config.put(fhicl_key(debug_table, "printMode"), mode);
  }

  std::string graph_option{"data-dependency-graph"};
  if (vm.count(graph_option)) {
    raw_config.put("services.scheduler.dataDependencyGraph",
                   vm[graph_option].as<std::string>());
  }

  if (vm.count("trace")) {
    raw_config.put("services.scheduler.wantTracer", true);
  } else if (vm.count("notrace")) {
    raw_config.put("services.scheduler.wantTracer", false);
  }
  auto const timingdb = vm.count("timing-db");
  if (vm.count("timing") || timingdb) {
    raw_config.putEmptyTable("services.TimeTracker");
    if (timingdb)
      raw_config.put("services.TimeTracker.dbOutput.filename",
                     vm["timing-db"].as<std::string>());
  } else if (vm.count("notiming")) {
    raw_config.erase("services.TimeTracker");
  }
  auto const memdb = vm.count("memcheck-db");
  if (vm.count("memcheck") || memdb) {
    raw_config.putEmptyTable("services.MemoryTracker");
    if (memdb)
      raw_config.put("services.MemoryTracker.dbOutput.filename",
                     vm["memcheck-db"].as<std::string>());
  } else if (vm.count("nomemcheck")) {
    raw_config.erase("services.MemoryTracker");
  }

  // messagefacility configuration.
  auto const message_key = fhicl_key("services", "message");
  auto const dests_key = fhicl_key(message_key, "destinations");
  if (!detail::exists_outside_prolog(raw_config, message_key)) {
    raw_config.put(fhicl_key(dests_key, "STDOUT.categories.ArtReport.limit"),
                   100);
    raw_config.put(fhicl_key(dests_key, "STDOUT.categories.default.limit"), -1);
    raw_config.put(fhicl_key(dests_key, "STDOUT.type"), "cout");
    raw_config.put(fhicl_key(dests_key, "STDOUT.threshold"), "INFO");
  }
  assert(detail::exists_outside_prolog(raw_config, dests_key));
  auto const& dests = raw_config.get<table_t const&>(dests_key);

  // By default, suppress all logging of messages of MTdiagnostics category.
  for (auto const& p : dests) {
    std::string const& dest_name = p.first;
    if (dest_name == "statistics"s)
      continue; // statistics are special -- see below
    raw_config.put(
      fhicl_key(dests_key, dest_name, "categories.MTdiagnostics.limit"), 0);
  }

  // The statistics destination represents a table of named destinations.
  auto const& stats_dest_key = fhicl_key(dests_key, "statistics");
  if (detail::exists_outside_prolog(raw_config, stats_dest_key)) {
    auto const& stats_dests = raw_config.get<table_t const&>(stats_dest_key);
    for (auto const& p : stats_dests) {
      std::string const& dest_name = p.first;
      raw_config.put(
        fhicl_key(stats_dest_key, dest_name, "categories.MTdiagnostics.limit"),
        0);
    }
  }

  if (vm.count("mt-diagnostics") > 0) {
    auto const dest = vm["mt-diagnostics"].as<std::string>();
    std::regex const re_stdout{R"((STDOUT|cout))",
                               std::regex_constants::ECMAScript |
                                 std::regex_constants::icase};
    std::regex const re_stderr{R"((STDERR|cerr))",
                               std::regex_constants::ECMAScript |
                                 std::regex_constants::icase};
    if (std::regex_match(dest, re_stdout)) {
      // Special handling since the 'cout' destination is already the
      // default per above.
      raw_config.put(
        fhicl_key(dests_key, "STDOUT.categories.MTdiagnostics.limit"), -1);
      return 0;
    }

    auto const mt_dest_key = fhicl_key(dests_key, "MTdiagnostics");
    if (std::regex_match(dest, re_stderr)) {
      raw_config.put(fhicl_key(mt_dest_key, "type"), "cerr");
    } else {
      raw_config.put(fhicl_key(mt_dest_key, "type"), "file");
      raw_config.put(fhicl_key(mt_dest_key, "filename"), dest);
    }
    raw_config.put(fhicl_key(mt_dest_key, "categories.MTdiagnostics.limit"),
                   -1);
    raw_config.put(fhicl_key(mt_dest_key, "categories.default.limit"), 0);
  }

  return 0;
}
