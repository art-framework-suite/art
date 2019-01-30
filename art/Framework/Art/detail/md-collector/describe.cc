#include "art/Framework/Art/detail/md-collector/describe.h"
#include "fhiclcpp/types/detail/PrintAllowedConfiguration.h"
#include "fhiclcpp/types/detail/optional_parameter_message.h"

std::string
art::detail::describe(cet::exempt_ptr<fhicl::ConfigurationTable const> config,
                      std::string const& tab)
{
  std::ostringstream oss;
  if (config == nullptr) {
    oss << "\n" << tab << "[ None provided ]\n";
  } else {
    oss << '\n' << tab << fhicl::detail::optional_parameter_message() << '\n';
    fhicl::detail::PrintAllowedConfiguration pac{oss, false, tab};
    pac.walk_over(*config->parameter_base());
  }
  return oss.str();
}
