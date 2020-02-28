#include "art/Framework/Art/DebugOptionsHandler.h"

#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "art/Framework/Art/detail/output_to.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <string>
#include <tuple>

using namespace std::string_literals;
using art::detail::fhicl_key;
using table_t = fhicl::extended_value::table_t;

art::DebugOptionsHandler::DebugOptionsHandler(bpo::options_description& desc)
{
  bpo::options_description debug_options{"Debugging options"};
  auto options = debug_options.add_options();
  add_opt(options,
          "mt-diagnostics,M",
          bpo::value<std::string>(),
          "Log art-specific multi-threading diagnostics to "
          "the provided destination.");
  add_opt(options, "trace", "Activate tracing.");
  add_opt(options, "no-trace", "Deactivate tracing.");
  add_opt(
    options, "timing", "Activate monitoring of time spent per event/module.");
  add_opt(options,
          "timing-db",
          bpo::value<std::string>(),
          "Output time-tracking data to SQLite3 database with name <db-file>.");
  add_opt(options, "no-timing", "Deactivate time tracking.");
  add_opt(options,
          "memcheck",
          "Activate monitoring of memory use (deprecated--per-job "
          "memory information printed in job summary).");
  add_opt(options,
          "memcheck-db",
          bpo::value<std::string>(),
          "Output memory use data to SQLite3 database with name <db-file>.");
  add_opt(options, "no-memcheck", "Deactivate monitoring of memory use.");
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
  add_opt(options,
          "debug-config",
          bpo::value<std::string>(),
          "Output post-processed configuration to <file> and exit.");
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
  if (vm.count("memcheck")) {
    std::cerr << "\nart warning: The '--memcheck' option is deprecated.  It "
                 "will be removed in art 3.04.\n"
                 "             Please remove it from the command-line as "
                 "memory information is printed in\n"
                 "             the end-of-job summary.\n\n";
  }
  if (vm.count("trace") + vm.count("no-trace") > 1) {
    throw Exception(errors::Configuration)
      << "Options --trace and --no-trace are incompatible.\n";
  }
  if (vm.count("timing") + vm.count("no-timing") > 1) {
    throw Exception(errors::Configuration)
      << "Options --timing and --no-timing are incompatible.\n";
  }
  if (vm.count("timing-db") + vm.count("no-timing") > 1) {
    throw Exception(errors::Configuration)
      << "Options --timing-db and --no-timing are incompatible.\n";
  }
  if (vm.count("memcheck-db") + vm.count("no-memcheck") > 1) {
    throw Exception(errors::Configuration)
      << "Options --memcheck-db and --no-memcheck are incompatible.\n";
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
  std::string const debug_table{fhicl_key(scheduler_key, "debug")};
  std::string option;
  std::string fn;

  // Remove any previously-defined "services.scheduler.debug" parameter.
  raw_config.erase(debug_table);
  raw_config.erase(
    fhicl_key(scheduler_key, "configOut")); // legacy configuration

  auto debugging_options = {"config-out", "debug-config", "validate-config"};
  for (auto const opt : debugging_options) {
    if (vm.count(opt)) {
      tie(option, fn) = make_tuple(opt, vm[opt].as<std::string>());
      break;
    }
  }

  std::string mode{"raw"};
  if (vm.count("annotate")) {
    mode = "annotate";
  }
  if (vm.count("prefix-annotate")) {
    mode = "prefix-annotate";
  }

  if (!option.empty()) {
    raw_config.put(fhicl_key(debug_table, "option"), option);
    raw_config.put(fhicl_key(debug_table, "fileName"), fn);
    raw_config.put(fhicl_key(debug_table, "printMode"), mode);
  }

  std::string graph_option{"data-dependency-graph"};
  if (vm.count(graph_option)) {
    raw_config.put("services.scheduler.dataDependencyGraph",
                   vm[graph_option].as<std::string>());
  }

  if (vm.count("trace")) {
    raw_config.putEmptyTable("services.Tracer");
  } else if (vm.count("no-trace")) {
    raw_config.erase("services.Tracer");
  }
  auto const timingdb = vm.count("timing-db");
  if (vm.count("timing") || timingdb) {
    raw_config.putEmptyTable("services.TimeTracker");
    if (timingdb)
      raw_config.put("services.TimeTracker.dbOutput.filename",
                     vm["timing-db"].as<std::string>());
  } else if (vm.count("no-timing")) {
    raw_config.erase("services.TimeTracker");
  }
  if (vm.count("memcheck-db")) {
    raw_config.put("services.MemoryTracker.dbOutput.filename",
                   vm["memcheck-db"].as<std::string>());
  } else if (vm.count("no-memcheck")) {
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
    if (detail::output_to_stdout(dest)) {
      // Special handling since the 'cout' destination is already the
      // default per above.
      raw_config.put(
        fhicl_key(dests_key, "STDOUT.categories.MTdiagnostics.limit"), -1);
      return 0;
    }

    auto const mt_dest_key = fhicl_key(dests_key, "MTdiagnostics");
    if (detail::output_to_stderr(dest)) {
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
