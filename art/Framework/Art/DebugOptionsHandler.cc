#include "art/Framework/Art/DebugOptionsHandler.h"

#include "art/Utilities/bold_fontify.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <string>

using namespace std::string_literals;
using art::detail::fhicl_key;

art::DebugOptionsHandler::
DebugOptionsHandler(bpo::options_description& desc,
                    std::string const& basename,
                    detail::DebugOutput& dbg)
  : dbg_{dbg}
{
  std::string const description {detail::bold_fontify("Debugging options")};
  bpo::options_description debug_options {description};
  debug_options.add_options()
    ("trace", "Activate tracing.")
    ("notrace", "Deactivate tracing.")
    ("timing", "Activate monitoring of time spent per event/module.")
    ("timing-db", bpo::value<std::string>(), "Output time-tracking data to SQLite3 database with name <db-file>.")
    ("notiming", "Deactivate time tracking.")
    ("memcheck", "Activate monitoring of memory use.")
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
art::DebugOptionsHandler::
doCheckOptions(bpo::variables_map const & vm)
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
art::DebugOptionsHandler::
doProcessOptions(bpo::variables_map const & vm,
                 fhicl::intermediate_table & raw_config)
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
    dbg_.set_filename( vm["debug-config"].as<std::string>() );
  }
  else if (vm.count("config-out")) {
    auto fn = vm["config-out"].as<std::string>().c_str();
    raw_config.put("services.scheduler.configOut",fn);
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
                     vm["timing-db"].as<std::string>().data());
  }
  else if (vm.count("notiming")) {
    raw_config.erase("services.TimeTracker");
  }
  auto const memdb = vm.count("memcheck-db");
  if (vm.count("memcheck") || memdb) {
    raw_config.putEmptyTable("services.MemoryTracker");
    if (memdb)
      raw_config.put("services.MemoryTracker.dbOutput.filename",
                     vm["memcheck-db"].as<std::string>().data());
  }
  else if (vm.count("nomemcheck")) {
    raw_config.erase("services.MemoryTracker");
  }
  return 0;
}
