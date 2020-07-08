#ifndef art_Framework_Art_detail_print_config_summary_h
#define art_Framework_Art_detail_print_config_summary_h

#include "art/Framework/Core/detail/EnabledModules.h"
#include "fhiclcpp/fwd.h"

#include <string>

namespace art::detail {
  void print_config_summary(fhicl::ParameterSet const& pset,
                            std::string const& verbosity,
                            EnabledModules const& enabled_modules);

  // Supporting functions
  void print_table_numbers(fhicl::ParameterSet const& pset,
                           std::string const& header);
  void print_path_numbers(EnabledModules const& enabled_modules);
  void print_path_names(EnabledModules const& enabled_modules);
  void print_service_types(fhicl::ParameterSet const& pset);
  void print_module_types(fhicl::ParameterSet const& pset,
                          std::string const& header);
}

#endif /* art_Framework_Art_detail_print_config_summary_h */

// Local Variables:
// mode: c++
// End:
