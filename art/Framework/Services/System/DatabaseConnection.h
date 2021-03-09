#ifndef art_Framework_Services_System_DatabaseConnection_h
#define art_Framework_Services_System_DatabaseConnection_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceDeclarationMacros.h"
#include "cetlib/sqlite/ConnectionFactory.h"
#include "cetlib/sqlite/detail/DefaultDatabaseOpenPolicy.h"

#include <string>
#include <utility>

namespace fhicl {
  class ParameterSet;
} // namespace fhicl

namespace art {

  class DatabaseConnection {
  public:
    explicit DatabaseConnection(fhicl::ParameterSet const&) {}
    template <typename DatabaseOpenPolicy =
                cet::sqlite::detail::DefaultDatabaseOpenPolicy,
              typename... PolicyArgs>
    cet::sqlite::Connection*
    get(std::string const& filename, PolicyArgs&&... policyArgs)
    {
      return factory_.make_connection<DatabaseOpenPolicy>(
        filename, std::forward<PolicyArgs>(policyArgs)...);
    }

  private:
    cet::sqlite::ConnectionFactory factory_;
  };

} // namespace art

DECLARE_ART_SERVICE(art::DatabaseConnection, SHARED)
#endif /* art_Framework_Services_System_DatabaseConnection_h */

// Local Variables:
// mode: c++
// End:
