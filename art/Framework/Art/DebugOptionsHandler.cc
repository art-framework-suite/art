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

art::DebugOptionsHandler::DebugOptionsHandler(bpo::options_description& desc,
                                              std::string const& basename,
                                              detail::DebugOutput& dbg)
  : dbg_{dbg}
{
  bpo::options_description debug_options{"Debugging options"};
  debug_options.add_options()
    ("mt-diagnostics,M", bpo::value<std::string>(), "Log art-specific multi-threading diagnostics to the provided destination.")
    ("trace", "Activate tracing.")
    ("notrace", "Deactivate tracing.")
    ("timing", "Activate monitoring of time spent per event/module.")
    ("timing-db", bpo::value<std::string>(), "Output time-tracking data to SQLite3 database with name <db-file>.")
    ("notiming", "Deactivate time tracking.")
    ("memcheck", "Activate monitoring of memory use (deprecated--per-job memory information printed in job summary).")
    ("memcheck-db", bpo::value<std::string>(), "Output memory use data to SQLite3 database with name <db-file>.")
    ("nomemcheck", "Deactivate monitoring of memory use.")
    ("debug-config", bpo::value<std::string>(),
     ("Output post-processed configuration to <file> and exit. Equivalent to env ART_DEBUG_CONFIG=<file> "s + basename + " ...").c_str())
    ("config-out", bpo::value<std::string>(),
     "Output post-processed configuration to <file> and continue with job.")
    ("annotate","Include configuration parameter source information.")
    ("prefix-annotate","Include configuration parameter source information on line preceding parameter declaration.");
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
  if (vm.count("debug-config") + vm.count("config-out") > 1 ) {
    throw Exception(errors::Configuration)
      << "Options --debug-config and --config-out are incompatible.\n";
  }
  if (vm.count("annotate") + vm.count("prefix-annotate") > 1){
    throw Exception(errors::Configuration)
      << "Options --annotate and --prefix-annotate are incompatible.\n";
  }

  if (vm.count("annotate") + vm.count("prefix-annotate") == 1 &&
      vm.count("debug-config") + vm.count("config-out") == 0 ){
    throw Exception(errors::Configuration) <<
      "Options --annotate and --prefix-annotate must be specified with "
      "either --debug-config or --config-out.\n";
  }

  return 0;
}

int
art::DebugOptionsHandler::doProcessOptions(bpo::variables_map const& vm,
                                           fhicl::intermediate_table& raw_config)
{

  using detail::DebugOutput;
  using dest_t = DebugOutput::destination;

  std::string fn;
  switch( DebugOutput::destination_via_env(fn) ) {
  case dest_t::cerr: dbg_.to_cerr()       ; break;
  case dest_t::file: dbg_.set_filename(fn); break;
  case dest_t::none: {}
  }

  // "debug-config" wins over ART_DEBUG_CONFIG
  if (vm.count("debug-config")) {
    dbg_.set_filename(vm["debug-config"].as<std::string>());
  }
  else if (vm.count("config-out")) {
    auto fn = vm["config-out"].as<std::string>();
    raw_config.put("services.scheduler.configOut", fn);
    dbg_.set_filename(fn);
    dbg_.set_preempting(false);
  }
  using namespace fhicl::detail;
  if (vm.count("annotate")){
    dbg_.set_mode(print_mode::annotated);
  }
  if (vm.count("prefix-annotate")){
    dbg_.set_mode(print_mode::prefix_annotated);
  }
  if (vm.count("trace")) {
    raw_config.put("services.scheduler.wantTracer", true);
  }
  else if (vm.count("notrace")) {
    raw_config.put("services.scheduler.wantTracer", false);
  }
  auto const timingdb = vm.count("timing-db");
  if (vm.count("timing") || timingdb) {
    raw_config.putEmptyTable("services.TimeTracker");
    if (timingdb)
      raw_config.put("services.TimeTracker.dbOutput.filename",
                     vm["timing-db"].as<std::string>());
  }
  else if (vm.count("notiming")) {
    raw_config.erase("services.TimeTracker");
  }
  auto const memdb = vm.count("memcheck-db");
  if (vm.count("memcheck") || memdb) {
    raw_config.putEmptyTable("services.MemoryTracker");
    if (memdb)
      raw_config.put("services.MemoryTracker.dbOutput.filename",
                     vm["memcheck-db"].as<std::string>());
  }
  else if (vm.count("nomemcheck")) {
    raw_config.erase("services.MemoryTracker");
  }

  // messagefacility configuration.
  auto const message_key = fhicl_key("services", "message");
  auto const dests_key = fhicl_key(message_key, "destinations");
  if (!detail::exists_outside_prolog(raw_config, message_key)) {
    raw_config.put(fhicl_key(dests_key, "STDOUT.categories.ArtReport.limit"), 100);
    raw_config.put(fhicl_key(dests_key, "STDOUT.categories.default.limit"), -1);
    raw_config.put(fhicl_key(dests_key, "STDOUT.type"), "cout");
    raw_config.put(fhicl_key(dests_key, "STDOUT.threshold"), "INFO");
  }
  assert(detail::exists_outside_prolog(raw_config, dests_key));
  auto const& dests = raw_config.get<table_t const&>(dests_key);

  // By default, suppress all logging of messages of MTdiagnostics category.
  for (auto const& p : dests) {
    std::string const& dest_name = p.first;
    raw_config.put(fhicl_key(dests_key, dest_name, "categories.MTdiagnostics.limit"), 0);
  }

  if (vm.count("mt-diagnostics") > 0) {
    auto const dest = vm["mt-diagnostics"].as<std::string>();
    std::regex const re_stdout{R"((STDOUT|cout))", std::regex_constants::ECMAScript | std::regex_constants::icase};
    std::regex const re_stderr{R"((STDERR|cerr))", std::regex_constants::ECMAScript | std::regex_constants::icase};
    if (std::regex_match(dest, re_stdout)) {
      // Special handling since the 'cout' destination is already the
      // default per above.
      raw_config.put(fhicl_key(dests_key, "STDOUT.categories.MTdiagnostics.limit"), -1);
      return 0;
    }

    auto const mt_dest_key = fhicl_key(dests_key, "MTdiagnostics");
    if (std::regex_match(dest, re_stderr)) {
      raw_config.put(fhicl_key(mt_dest_key, "type"), "cerr");
    }
    else {
      raw_config.put(fhicl_key(mt_dest_key, "type"), "file");
      raw_config.put(fhicl_key(mt_dest_key, "filename"), dest);
    }
    raw_config.put(fhicl_key(mt_dest_key, "categories.MTdiagnostics.limit"), -1);
    raw_config.put(fhicl_key(mt_dest_key, "categories.default.limit"), 0);
  }

  return 0;
}
