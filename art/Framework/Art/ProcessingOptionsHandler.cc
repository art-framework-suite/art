#include "art/Framework/Art/ProcessingOptionsHandler.h"

#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/bold_fontify.h"
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
            bool const default_value)
  {
    if (vm.count(bpo_key)) {
      config.put(fhicl_key, vm[bpo_key].as<bool>());
    } else if (!exists_outside_prolog(config, fhicl_key)) {
      config.put(fhicl_key, default_value);
    }
  }

} // namespace

art::ProcessingOptionsHandler::ProcessingOptionsHandler(
  bpo::options_description& desc)
{
  bpo::options_description processing_options{"Processing options"};
  auto options = processing_options.add_options();
  add_opt(options,
          "parallelism,j",
          bpo::value<int>(),
          "Number of threads AND schedules to use for event processing "
          "(default = 1, 0 = all cores).");
  add_opt(options,
          "nschedules",
          bpo::value<int>(),
          "Number of schedules to use for event processing (default = 1)");
  // Note: tbb wants nthreads to be an int!
  add_opt(options,
          "nthreads",
          bpo::value<int>(),
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

  // 'parallelism' is incompatible with either 'nthreads' or
  // 'nschedules'.
  if (vm.count("parallelism")) {
    if (vm.count("nthreads") or vm.count("nschedules")) {
      throw Exception(errors::Configuration) << "The -j/--parallelism option "
                                                "cannot be used with either "
                                                "--nthreads or --nschedules.\n";
    }
  }

  if (vm.count("nthreads") and vm["nthreads"].as<int>() < 0) {
    throw Exception(errors::Configuration)
      << "Option --nthreads must greater than or equal to 0.";
  }
  if (vm.count("nschedules") and vm["nschedules"].as<int>() < 1) {
    throw Exception(errors::Configuration)
      << "Option --nschedules must be at least 1.\n";
  }
  return 0;
}

int
art::ProcessingOptionsHandler::doProcessOptions(
  bpo::variables_map const& vm,
  fhicl::intermediate_table& raw_config)
{
  auto const scheduler_key = fhicl_key("services", "scheduler");

  if (vm.count("rethrow-all") == 1 || vm.count("rethrow-default") == 1) {
    raw_config.put(fhicl_key(scheduler_key, "defaultExceptions"), false);
    if (vm.count("rethrow-all") == 1) {
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "IgnoreCompletely"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "SkipEvent"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "FailModule"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "FailPath"));
    }
  }

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

  auto const num_schedules_key = fhicl_key(scheduler_key, "num_schedules");
  auto const num_threads_key = fhicl_key(scheduler_key, "num_threads");
  if (vm.count("parallelism")) {
    // 'nthreads' and 'nschedules' are set to the same value.
    auto const j = vm["parallelism"].as<int>();
    auto const nthreads =
      (j == 0) ? tbb::task_scheduler_init::default_num_threads() : j;
    raw_config.put(num_schedules_key, nthreads);
    raw_config.put(num_threads_key, nthreads);
    return 0;
  }

  if (vm.count("nschedules")) {
    raw_config.put(num_schedules_key, vm["nschedules"].as<int>());
  }
  if (vm.count("nthreads")) {
    auto const nt = vm["nthreads"].as<int>();
    auto const nthreads =
      (nt == 0) ? tbb::task_scheduler_init::default_num_threads() : nt;
    raw_config.put(num_threads_key, nthreads);
  }

  // If 'nschedules' or 'nthreads' does not exist in configuration,
  // assign the default value of 1.
  if (not exists_outside_prolog(raw_config, num_schedules_key)) {
    raw_config.put(num_schedules_key, 1);
  }
  if (not exists_outside_prolog(raw_config, num_threads_key)) {
    raw_config.put(num_threads_key, 1);
  }

  return 0;
}
