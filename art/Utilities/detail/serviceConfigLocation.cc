#include "art/Utilities/detail/serviceConfigLocation.h"
#include "fhiclcpp/intermediate_table.h"

std::string
art::detail::
serviceConfigLocation(fhicl::intermediate_table & raw_config,
                      std::string const & service)
{
  return (raw_config.exists(std::string("services.user.") +
                            service) ?
          "services.user." : "services.") + service;
}
