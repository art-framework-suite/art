#include "art/Framework/Art/EventProcessingOptionsHandler.h"

#include "art/Framework/Art/detail/bold_fontify.h"
#include "art/Framework/Art/detail/exists_outside_prolog.h"
#include "art/Framework/Art/detail/fhicl_key.h"
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/coding.h"
#include "fhiclcpp/extended_value.h"
#include "fhiclcpp/intermediate_table.h"
#include "fhiclcpp/parse.h"

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

  void fillTable( std::string const& bpo_key,
                  std::string const& fhicl_key,
                  bpo::variables_map const& vm,
                  fhicl::intermediate_table& config )
  {
    if (vm.count(bpo_key))
      config.put(fhicl_key, vm[bpo_key].as<bool>());
    else if (!exists_outside_prolog(config, fhicl_key))
      config.put(fhicl_key, true);
  }

}

art::EventProcessingOptionsHandler::
EventProcessingOptionsHandler(bpo::options_description& desc,
                    bool const rethrowDefault)
  : rethrowDefault_{rethrowDefault}
{
  std::string const description {detail::bold_fontify("Event-processing options")};
  bpo::options_description processing_options {description};
  processing_options.add_options()
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
    ;
  desc.add(processing_options);
}

int
art::EventProcessingOptionsHandler::
doCheckOptions(bpo::variables_map const & vm)
{
  if ((vm.count("rethrow-all") +
       vm.count("rethrow-default") +
       vm.count("no-rethrow-default")) > 1) {
    throw Exception(errors::Configuration)
      << "Options --default-exceptions, --rethrow-all and --rethrow-default \n"
      << "are mutually incompatible.\n";
  }
  return 0;
}

int
art::EventProcessingOptionsHandler::
doProcessOptions(bpo::variables_map const & vm,
                 fhicl::intermediate_table & raw_config)
{
  std::string const scheduler_key {"services.scheduler"};

  if (vm.count("rethrow-all") == 1 ||
      vm.count("rethrow-default") == 1 ||
      (rethrowDefault_ && vm.count("default-exceptions") == 0) ) {
    raw_config.put(fhicl_key(scheduler_key, "defaultExceptions"), false);
    if (vm.count("rethrow-all") == 1) {
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "IgnoreCompletely"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "SkipEvent"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "FaileModule"));
      raw_config.putEmptySequence(fhicl_key(scheduler_key, "FailPath"));
    }
  }

  fillTable("errorOnFailureToPut", fhicl_key(scheduler_key, "errorOnFailureToPut"), vm, raw_config);
  fillTable("errorOnSIGINT"      , fhicl_key(scheduler_key, "errorOnSIGINT"      ), vm, raw_config);
  return 0;
}
