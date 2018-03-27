#include "art/Framework/Art/ProcessingOptionsHandler.h"

#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"
#include "tbb/task_scheduler_init.h"

#include <string>

using namespace std::string_literals;
using art::detail::fhicl_key;

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

  void
  fillTable(std::string const& bpo_key,
            std::string const& fhicl_key,
            bpo::variables_map const& vm,
            fhicl::intermediate_table& config,
            bool const flag_value)
  {
    if (vm.count(bpo_key)) {
      config.put(fhicl_key, vm[bpo_key].as<bool>());
    } else if (!exists_outside_prolog(config, fhicl_key)) {
      config.put(fhicl_key, flag_value);
    }
  }

} // namespace

art::ProcessingOptionsHandler::ProcessingOptionsHandler(
  bpo::options_description& desc,
  bool const rethrowDefault)
  : rethrowDefault_{rethrowDefault}
{

  bpo::options_description processing_options{"Processing options"};
  auto options = processing_options.add_options();
  // Note: tbb wants nthreads to be an int!
  add_opt(options,
          "nthreads",
          bpo::value<int>()->default_value(1),
          "Number of threads to use for event processing (default = 1, 0 = all "
          "cores)");
  add_opt(options,
          "default-exceptions",
          "Some exceptions may be handled differently by default (e.g. "
          "ProductNotFound).");
  add_opt(options, "rethrow-default", "All exceptions default to rethrow.");
  add_opt(options,
          "rethrow-all",
          "All exceptions overridden to rethrow (cf rethrow-default).");
  add_opt(
    options,
    "errorOnFailureToPut",
    bpo::value<bool>()->implicit_value(true, "true"),
    "Global flag that controls the behavior upon failure to 'put' a "
    "product (declared by 'produces') onto the Event.  If 'true', per-module "
    "flags can override the value of the global flag.");
  add_opt(
    options,
    "errorOnMissingConsumes",
    bpo::value<bool>()->implicit_value(true, "true"),
    "If 'true', then an exception will be thrown if any module attempts "
    "to retrieve a product via the 'getBy*' interface without specifying "
    "the appropriate 'consumes<T>(...)' statement in the module constructor.");
  add_opt(
    options,
    "errorOnSIGINT",
    bpo::value<bool>()->implicit_value(true, "true"),
    "If 'true', a signal received from the user yields an art return code "
    "corresponding to an error; otherwise return 0.");
  desc.add(processing_options);
}

int
art::ProcessingOptionsHandler::doCheckOptions(bpo::variables_map const& vm)
{
  if ((vm.count("rethrow-all") + vm.count("rethrow-default") +
       vm.count("no-rethrow-default")) > 1) {
    throw Exception(errors::Configuration)
      << "Options --default-exceptions, --rethrow-all, and --rethrow-default "
         "\n"
      << "are mutually incompatible.\n";
  }

  // No need to check for presence of 'nthreads' since it has a
  // default value.
  if (vm["nthreads"].as<int>() < 0) {
    throw Exception(errors::Configuration)
      << "Option --nthreads must greater than or equal to 0.";
  }
  return 0;
}

int
art::ProcessingOptionsHandler::doProcessOptions(
  bpo::variables_map const& vm,
  fhicl::intermediate_table& raw_config)
{
  std::string const scheduler_key{"services.scheduler"};

  if (vm.count("rethrow-all") == 1 || vm.count("rethrow-default") == 1 ||
      (rethrowDefault_ && vm.count("default-exceptions") == 0)) {
    raw_config.put(fhicl_key(scheduler_key, "defaultExceptions"), false);
    if (vm.count("rethrow-all") == 1) {
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "IgnoreCompletely"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "SkipEvent"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "FailModule"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "FailPath"));
    }
  }

  fillTable("errorOnFailureToPut",
            fhicl_key(scheduler_key, "errorOnFailureToPut"),
            vm,
            raw_config,
            true);
  fillTable("errorOnMissingConsumes",
            fhicl_key(scheduler_key, "errorOnMissingConsumes"),
            vm,
            raw_config,
            false);
  fillTable("errorOnSIGINT",
            fhicl_key(scheduler_key, "errorOnSIGINT"),
            vm,
            raw_config,
            true);

  auto const nt = vm["nthreads"].as<int>();
  auto const nthreads =
    (nt == 0) ? tbb::task_scheduler_init::default_num_threads() : nt;
  raw_config.put(fhicl_key(scheduler_key, "num_threads"), nthreads);
  return 0;
}
