#include "art/Framework/Art/DebugOptionsHandler.h"

#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Utilities/Exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

#include <string>

using namespace std::string_literals;

namespace {

  using art::detail::exists_outside_prolog;

  // For 'fillTable' the behavior is as follows:
  //
  // (1) If the program option is specified at the command line, the
  //     corresponding FHiCL parameter is added to the intermediate
  //     table (if it doesn't already exist), or the corresponding
  //     value is overwritten (if the parameter does already exist).
  //
  // (2) If the program option is not specified AND the FHiCL file
  //     does not have the corresponding parameter, then a default
  //     value of 'true' is added to the FHiCL configuration.
  //
  // This function could be made more general, but there is currently
  // no need.

  void fillTable( std::string const& bpo_key,
                  std::string const& fhicl_key,
                  bpo::variables_map const& vm,
                  fhicl::intermediate_table& config )
  {
    if ( vm.count( bpo_key ) )
      config.put( fhicl_key, vm[bpo_key].as<bool>() );
    else if ( !exists_outside_prolog(config, fhicl_key) )
      config.put( fhicl_key, true );
  }

}

art::DebugOptionsHandler::
DebugOptionsHandler(bpo::options_description & desc,
                    std::string const & basename,
                    detail::DebugOutput& dbg,
                    bool rethrowDefault)
  : dbg_(dbg)
  , rethrowDefault_(rethrowDefault)
{
  desc.add_options()
    ("trace", "Activate tracing.")
    ("notrace", "Deactivate tracing.")
    ("memcheck", "Activate monitoring of memory use.")
    ("memcheck-db", bpo::value<std::string>(), "Output memory use data to SQLite3 database with name <db-file>.")
    ("nomemcheck", "Deactivate monitoring of memory use.")
    ("default-exceptions", "Some exceptions may be handled differently by default (e.g. ProductNotFound).")
    ("rethrow-default", "All exceptions default to rethrow.")
    ("rethrow-all", "All exceptions overridden to rethrow (cf rethrow-default).")
    ("errorOnFailureToPut", bpo::value<bool>()->implicit_value(true,"true"),
     "Global flag that controls the behavior upon failure to 'put' a product "
     "(declared by 'produces') onto the Event.  If 'true', per-module flags "
     "can override the value of the global flag.")
    ("errorOnSIGINT", bpo::value<bool>()->implicit_value(true,"true"),
     "If 'true', a signal received from the user yields an art return code "
     "corresponding to an error; otherwise return 0.")
    ("debug-config", bpo::value<std::string>(),
     ("Output post-processed configuration to <file> and exit. Equivalent to env ART_DEBUG_CONFIG=<file> "s + basename + " ..."s).c_str())
    ("config-out", bpo::value<std::string>(),
     "Output post-processed configuration to <file> and continue with job.")
    ("annotate","Include configuration parameter source information.")
    ("prefix-annotate","Include configuration parameter source information on line preceding parameter declaration.");
}

int
art::DebugOptionsHandler::
doCheckOptions(bpo::variables_map const & vm)
{
  if ((vm.count("rethrow-all") +
       vm.count("rethrow-default") +
       vm.count("no-rethrow-default")) > 1) {
    throw Exception(errors::Configuration)
      << "Options --default-exceptions, --rethrow-all and --rethrow-default \n"
      << "are mutually incompatible.\n";
  }
  if (vm.count("trace") + vm.count("notrace") > 1) {
    throw Exception(errors::Configuration)
      << "Options --trace and --notrace are incompatible.\n";
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
    dbg_.set_mode( print_mode::annotated );
  }
  if (vm.count("prefix-annotate")){
    dbg_.set_mode( print_mode::parsable );
  }
  if (vm.count("trace")) {
    raw_config.put("services.scheduler.wantTracer", true);
  }
  else if (vm.count("notrace")) {
    raw_config.put("services.scheduler.wantTracer", false);
  }
  auto const memdb = vm.count("memcheck-db");
  if (vm.count("memcheck") || memdb) {
    raw_config.putEmptyTable("services.MemoryTracker");
    if ( memdb )
      raw_config.put("services.MemoryTracker.filename",
                     vm["memcheck-db"].as<std::string>().data());
  }
  else if (vm.count("nomemcheck")) {
    raw_config.erase("services.SimpleMemoryCheck");
    raw_config.erase("services.MemoryTracker");
  }
  if (vm.count("rethrow-all") == 1 ||
      vm.count("rethrow-default") == 1 ||
      (rethrowDefault_ && vm.count("default-exceptions") == 0) ) {
    raw_config.put("services.scheduler.defaultExceptions", false);
    if (vm.count("rethrow-all") == 1) {
      raw_config.putEmptySequence("services.scheduler.IgnoreCompletely");
      raw_config.putEmptySequence("services.scheduler.SkipEvent");
      raw_config.putEmptySequence("services.scheduler.FaileModule");
      raw_config.putEmptySequence("services.scheduler.FailPath");
    }
  }

  std::string const fhicl_base = "services.scheduler."s;
  fillTable("errorOnFailureToPut", fhicl_base+"errorOnFailureToPut", vm, raw_config );
  fillTable("errorOnSIGINT"      , fhicl_base+"errorOnSIGINT"      , vm, raw_config );

  return 0;
}
