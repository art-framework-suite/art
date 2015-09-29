#ifndef art_Framework_EventProcessor_ServiceDirector_h
#define art_Framework_EventProcessor_ServiceDirector_h
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Utilities/Exception.h"
#include "cetlib/demangle.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <memory>

namespace art {
  class ServiceDirector;
}

class art::ServiceDirector {
public:
  ServiceDirector(fhicl::ParameterSet services,
                  ActivityRegistry & areg,
                  ServiceToken & token);
  template <typename SERVICE, typename ... ARGS>
  void addSystemService(ARGS&& ... args);

private:
  ServiceToken & serviceToken_;
};

#ifndef __GCCXML__
template <typename SERVICE, typename ... ARGS>
void
art::ServiceDirector::
addSystemService(ARGS&& ... args)
{
  try {
    serviceToken_.add(std::make_unique<SERVICE>( std::forward<ARGS>(args)... ) );
  }
  catch (fhicl::detail::validationException const & e) {
    std::ostringstream err_stream;
    std::size_t const width (100);
    err_stream << "\n"
               << std::string(width,'=')
               << "\n\n"
               << "!! The following service has been misconfigured: !!"
               << "\n\n"
               << std::string(width,'-')
               << "\n\nservice_type: \033[1m"
               << cet::demangle_symbol( typeid(SERVICE).name() )
               << "\033[0m"
               << "\n\n" << e.what()
               << "\n"
               << std::string(width,'=')
               << "\n\n";
    throw art::Exception(art::errors::Configuration) << err_stream.str();
  }
}
#endif

#endif /* art_Framework_EventProcessor_ServiceDirector_h */

// Local Variables:
// mode: c++
// End:
