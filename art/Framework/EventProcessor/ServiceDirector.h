#ifndef art_Framework_EventProcessor_ServiceDirector_h
#define art_Framework_EventProcessor_ServiceDirector_h
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Utilities/HorizontalRule.h"
#include "art/Utilities/bold_fontify.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <memory>

namespace art {
  class ServiceDirector;
}

class art::ServiceDirector {
public:

  explicit ServiceDirector(fhicl::ParameterSet&& services,
                           ActivityRegistry& areg,
                           ServiceToken& token);

  template <typename SERVICE, typename... ARGS>
  void addSystemService(ARGS&& ... args);

private:
  ServiceToken& serviceToken_;
};

template <typename SERVICE, typename... ARGS>
void
art::ServiceDirector::
addSystemService(ARGS&&... args)
try {
  serviceToken_.add(std::make_unique<SERVICE>(std::forward<ARGS>(args)...));
}
catch (fhicl::detail::validationException const& e) {
  constexpr HorizontalRule rule{100};
  throw art::Exception(art::errors::Configuration)
    << "\n"
    << rule('=')
    << "\n\n"
    << "!! The following service has been misconfigured: !!"
    << "\n\n"
    << rule('-')
    << "\n\nservice_type: " << art::detail::bold_fontify(cet::demangle_symbol(typeid(SERVICE).name()))
    << "\n\n" << e.what()
    << "\n"
    << rule('=')
    << "\n\n";
 }

#endif /* art_Framework_EventProcessor_ServiceDirector_h */

// Local Variables:
// mode: c++
// End:
