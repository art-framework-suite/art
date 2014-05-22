#ifndef art_Framework_Services_Registry_ServiceRegistry_h
#define art_Framework_Services_Registry_ServiceRegistry_h

// ======================================================================
//
// ServiceRegistry - Manages the 'thread specific' instance of Services
//
// ======================================================================

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/ServiceToken.h"
#include "art/Framework/Services/Registry/ServicesManager.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "cetlib/LibraryManager.h"
#include "art/Utilities/ScheduleID.h"

#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

// ----------------------------------------------------------------------

namespace art {
  class ActivityRegistry;

  class ServiceRegistry
  {
    // non-copyable:
    ServiceRegistry( ServiceRegistry const & ) = delete;
    ServiceRegistry& operator=( ServiceRegistry const & ) = delete;

  public:
    class Operate
    {
      // non-copyable:
      Operate( Operate const & );
      void  operator = ( Operate const & );

      // override operator new to stop use on heap?

    public:
      // c'tor:
      Operate(const ServiceToken & iToken)
      : oldToken_( ServiceRegistry::instance().setContext(iToken) )
      { }

      // d'tor:
      ~Operate()
      { ServiceRegistry::instance().unsetContext(oldToken_); }

    private:
      ServiceToken oldToken_;
    };  // Operate

    friend int main( int argc, char* argv[] );
    friend class Operate;

    virtual ~ServiceRegistry();

    template< class T, typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val != ServiceScope::PER_SCHEDULE>::type>
    T & get() const
      {
        if( ! manager_.get() )
          throw art::Exception(art::errors::NotFound, "Service")
            <<" no ServiceRegistry has been set for this thread";
        return manager_-> template get<T>();
      }

    template< class T, typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val == ServiceScope::PER_SCHEDULE>::type>
    T & get(ScheduleID sID) const
      {
        if( ! manager_.get() )
          throw art::Exception(art::errors::NotFound, "Service")
            <<" no ServiceRegistry has been set for this thread";
        return manager_-> template get<T>(sID);
      }

    template<class T> bool isAvailable() const
    {
      if( ! manager_.get() )
        throw art::Exception(art::errors::NotFound, "Service")
          <<" no ServiceRegistry has been set for this thread";
      return manager_-> template isAvailable<T>();
    }

     // The token can be passed to another thread in order to have the
     // same services available in the other thread.

    ServiceToken presentToken() const;

    static ServiceRegistry & instance();

  public: // Made public (temporarily) at the request of Emilio Meschi.
    typedef ServicesManager SM;
    typedef std::vector<fhicl::ParameterSet> ParameterSets;

    static ServiceToken createSet(ParameterSets const &, ActivityRegistry & );

  private:
    // returns old token
    ServiceToken setContext( ServiceToken const & iNewToken );
    void unsetContext( ServiceToken const & iOldToken );

    ServiceRegistry();

    // ---------- member data --------------------------------
    cet::LibraryManager lm_;
    std::shared_ptr<ServicesManager> manager_;

  };  // ServiceRegistry

}  // art

// ======================================================================

#endif /* art_Framework_Services_Registry_ServiceRegistry_h */

// Local Variables:
// mode: c++
// End:
