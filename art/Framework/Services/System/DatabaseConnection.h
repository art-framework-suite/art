#ifndef art_Framework_Services_System_DatabaseConnection_h
#define art_Framework_Services_System_DatabaseConnection_h

// ======================================================================
//
// DatabaseConnection
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "cetlib/sqlite/ConnectionFactory.h"
#include "cetlib/sqlite/detail/DefaultDatabaseOpenPolicy.h"

namespace fhicl {
  class ParameterSet;
}

namespace art {
  class DatabaseConnection {
  public:
    explicit DatabaseConnection(fhicl::ParameterSet const&) {}

    template <typename DatabaseOpenPolicy =
                cet::sqlite::detail::DefaultDatabaseOpenPolicy,
              typename... PolicyArgs>
    cet::sqlite::Connection
    get(std::string const& filename, PolicyArgs&&... policyArgs)
    {
      return factory_.make<DatabaseOpenPolicy>(
        filename, std::forward<PolicyArgs>(policyArgs)...);
    }

  private:
    cet::sqlite::ConnectionFactory factory_;
  };
}

DECLARE_ART_SERVICE(art::DatabaseConnection, LEGACY)
#endif /* art_Framework_Services_System_DatabaseConnection_h */

// Local Variables:
// mode: c++
// End:
