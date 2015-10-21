#include "art/Framework/Core/detail/verify_names.h"
#include "art/Utilities/Exception.h"

void
art::detail::verifyInstanceName(std::string const& instanceName)
{
  if (instanceName.find('_') != std::string::npos) {
    throw art::Exception(art::errors::Configuration)
      << "Instance name \""
      << instanceName
      << "\" is illegal: underscores are not permitted in instance names."
      << '\n';
  }
}

void
art::detail::verifyFriendlyClassName(std::string const& fcn)
{
  if (fcn.find('_') != std::string::npos) {
    throw art::Exception(art::errors::LogicError)
      << "Class \""
      << fcn
      << "\" is not suitable for use as a product due to the presence of "
      << "underscores which are not allowed anywhere in the class name "
      << "(including namespace and enclosing classes).\n";
  }
}
