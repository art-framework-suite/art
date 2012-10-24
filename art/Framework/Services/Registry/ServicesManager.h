#ifndef art_Framework_Services_Registry_ServicesManager_h
#define art_Framework_Services_Registry_ServicesManager_h

////////////////////////////////////////////////////////////////////////
// ServicesManager
//
// ======================================================================

#include "art/Utilities/LibraryManager.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/demangle.h"
#include "cetlib/trim.h"
#include "cpp0x/memory"
#include "cpp0x/utility"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <cassert>
#include <iomanip>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace art {

  class ServicesManager {
    // non-copyable:
    ServicesManager(ServicesManager const &);
    void operator = (ServicesManager const &);

  public:
    typedef std::vector<fhicl::ParameterSet> ParameterSets;

  private:
    typedef std::unique_ptr<detail::ServiceHelperBase> (*SHBCREATOR_t) ();

    typedef  std::shared_ptr<detail::ServiceWrapperBase>  WrapperBase_ptr;
    class Cache;

    typedef  std::map< TypeID, Cache >  Factory;
    typedef  std::map< std::string, Factory::iterator > NameIndex;
    typedef  std::vector< TypeID >      TypeIDs;
    typedef  std::stack< WrapperBase_ptr >  ServiceStack;

    // A Cache object encapsulates the (possibly delayed) creation of
    // a Service object.
    class Cache {
    public:
      // Service implementation.
      Cache(fhicl::ParameterSet const & pset,
            std::unique_ptr<detail::ServiceHelperBase> && helper)
        :
        config_(pset),
        helper_(std::move(helper)),
        service_(),
        interface_impl_()
        {
        }

      // Service interface implementation.
      Cache(fhicl::ParameterSet const & pset,
            std::unique_ptr<detail::ServiceHelperBase> && helper,
            Factory::iterator impl)
        :
        config_(pset),
        helper_(std::move(helper)),
        service_(),
        interface_impl_(impl)
        {
        }

      // Pre-made service (1).
      Cache(WrapperBase_ptr premade_service,
            std::unique_ptr<detail::ServiceHelperBase> && helper)
        :
        config_(),
        helper_(std::move(helper)),
        service_(premade_service),
        interface_impl_()
        {
        }

      // Create the service if necessary, and return the WrapperBase_ptr
      // that refers to it.
      WrapperBase_ptr getService(ActivityRegistry & reg, ServiceStack & creationOrder) const {
        if (is_interface()) { // Service interface
          if (!service_) {
            // No cached instance, we need to make it.
            if (!interface_impl_->second.service_) {
              // The service provider has no cached instance, have it make one.
              interface_impl_->second.createService(reg, creationOrder);
            }
            // Convert the service provider wrapper to a service interface wrapper,
            // and use that as our cached instance.
            interface_impl_->second.convertService(service_);
          }
        }
        else { // Concrete service.
          if (!service_) {
            // No cached instance, we need to make it.
            createService(reg, creationOrder);
          }
        }
        return service_;
      }

      void forceCreation(ActivityRegistry & reg) const {
        assert(is_impl() && "Cache::forceCreation called on a service interface!");
        if (!service_) {
          makeAndCacheService(reg);
        }
      }

      fhicl::ParameterSet const & getParameterSet() const {
        return config_;
      }

      void putParameterSet(fhicl::ParameterSet const & newConfig) {
        if (config_ != newConfig) {
          config_ = newConfig;
          if (service_) {
            service_->reconfigure(config_);
          }
        }
      }

      Factory::iterator getInterfaceImpl() const {
        return interface_impl_;
      }

      void makeAndCacheService(ActivityRegistry & reg) const {
        assert(is_impl() && "Cache::makeAndCacheService called on a service interface!");
        try {
          if (serviceScope() == ServiceScope::PER_SCHEDULE) {
            service_ = dynamic_cast<detail::ServicePSMHelper&>(*helper_).make(config_, reg, nSchedules());
          } else {
            service_ = dynamic_cast<detail::ServiceLGMHelper&>(*helper_).make(config_, reg);
          }
        }
        catch (cet::exception & e) {
          throw Exception(errors::OtherArt, "ServiceCreation", e)
              << "cet::exception caught during construction of service type "
              << cet::demangle_symbol(helper_->get_typeid().name())
              << ":\n";
        }
        catch (std::exception & e) {
          throw Exception(errors::StdException, "ServiceCreation")
              << "std::exception caught during construction of service type "
              << cet::demangle_symbol(helper_->get_typeid().name())
              << ": "
              << e.what();
        }
        catch (std::string const & s) {
          throw Exception(errors::BadExceptionType)
              << "String exception during construction of service type "
              << cet::demangle_symbol(helper_->get_typeid().name())
              << ": "
              << s;
        }
        catch (...) {
          throw Exception(errors::Unknown)
              << "String exception during construction of service type "
              << cet::demangle_symbol(helper_->get_typeid().name())
              << ":\n";
        }
      }

      void
      createService(ActivityRegistry & reg, ServiceStack & creationOrder) const {
        assert(is_impl() && "Cache::createService called on a service interface!");
        // When we actually create the Service object, we have to
        // remember the order of creation.
        makeAndCacheService(reg);
        creationOrder.push(service_);
      }

      void
      convertService(WrapperBase_ptr & swb) const {
        assert(is_impl() && "Cache::convertService called on a service interface!");
        swb = dynamic_cast<detail::ServiceInterfaceImplHelper&>(*helper_).convert(service_);
      }

      ServiceScope serviceScope() const { return helper_->scope(); }

      template <typename T,
                typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val != ServiceScope::PER_SCHEDULE>::type>
      T & get(ActivityRegistry & reg, ServiceStack & creationOrder) const {
        assert(serviceScope() != ServiceScope::PER_SCHEDULE &&
               "Called wrong service get() function: need ScheduleID");
        WrapperBase_ptr swb = getService(reg, creationOrder);
        return *reinterpret_cast<T *>(dynamic_cast<detail::ServiceLGRHelper&>(*helper_).retrieve(swb));
      }

      template <typename T,
                typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val == ServiceScope::PER_SCHEDULE>::type>
      T & get(ActivityRegistry & reg,
              ServiceStack & creationOrder,
              ScheduleID sID) const {
        assert(serviceScope() == ServiceScope::PER_SCHEDULE &&
               "Called wrong service get() function!");
        WrapperBase_ptr swb = getService(reg, creationOrder);
        return *reinterpret_cast<T *>(dynamic_cast<detail::ServicePSRHelper&>(*helper_).retrieve(swb, sID));
      }

      static void setNSchedules(size_t nSched) { nSchedules() = nSched; }

  private:
      static size_t & nSchedules() { static size_t s_ns = 1; return s_ns; }

      bool is_impl() const { return !is_interface(); }
      bool is_interface() const { return helper_->is_interface(); }

      fhicl::ParameterSet config_;
      std::unique_ptr<detail::ServiceHelperBase> helper_;
      mutable WrapperBase_ptr service_;
      Factory::iterator const interface_impl_;
    };  // Cache

  public:
    ServicesManager(ParameterSets const &,
                    LibraryManager const &,
                    ActivityRegistry &);

    ~ServicesManager();

    template <class T,
              typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val != ServiceScope::PER_SCHEDULE>::type>
    T &
    get();

    template <class T,
              typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val == ServiceScope::PER_SCHEDULE>::type>
    T &
    get(ScheduleID);

    //returns true of the particular service is accessible -- that is,
    // it either can be made (if requested) or has already been made.
    template< class T >
    bool
    isAvailable() const {
      return factory_.find(TypeID(typeid(T))) != factory_.end();
    }

    template <typename T,
              typename = typename std::enable_if<detail::ServiceHelper<T>::scope_val != ServiceScope::PER_SCHEDULE>::type>
    void
    put(std::unique_ptr<T> && premade_service) {
      std::unique_ptr<detail::ServiceHelperBase>
        service_helper(new detail::ServiceHelper<T>);
      TypeID id(typeid(T));
      Factory::const_iterator it = factory_.find(id);
      if (it != factory_.end()) {
        throw art::Exception(art::errors::LogicError, "Service")
          << "The system has manually added service of type "
          << cet::demangle_symbol(id.name())
          << ", but the service system already has a configured service"
          << " of that type\n";
      }
      WrapperBase_ptr swb(new detail::ServiceWrapper<T, detail::ServiceHelper<T>::scope_val>(std::move(premade_service)));
      actualCreationOrder_.push(swb);
      factory_.insert(std::make_pair(id, Cache(std::move(swb),
                                               std::move(service_helper))));
    }

    // SP<T> must be convertible to std::shared_ptr<T>.
    template <class SP,
              typename = typename std::enable_if<detail::ServiceHelper<typename SP::element_type>::scope_val == ServiceScope::PER_SCHEDULE>::type>
    void
    put(std::vector<SP> && premade_services) {
      typedef typename SP::element_type element_type;
      std::unique_ptr<detail::ServiceHelperBase>
        service_helper(new detail::ServiceHelper<element_type>);
      TypeID id(typeid(element_type));
      Factory::const_iterator it = factory_.find(id);
      if (it != factory_.end()) {
        throw art::Exception(art::errors::LogicError, "Service:")
          << "The system has manually added service of type "
          << cet::demangle_symbol(id.name())
          << ", but the service system already has a configured service"
          << " of that type\n";
      }
      WrapperBase_ptr swb(new detail::ServiceWrapper<element_type, detail::ServiceHelper<element_type>::scope_val>(std::move(premade_services)));
      actualCreationOrder_.push(swb);
      factory_.insert(std::make_pair(id, Cache(std::move(swb),
                                               std::move(service_helper))));
    }

    // force all the services that are not alrady made into existance
    // using 'reg'.  The order of creation will be the registration order.
    void forceCreation();

    void getParameterSets(ParameterSets & out) const;
    void putParameterSets(ParameterSets const &);

  private:
    void fillFactory_(ParameterSets const & psets, LibraryManager const & lm);

    std::pair<Factory::iterator, bool>
    insertImpl_(fhicl::ParameterSet const & pset,
                std::unique_ptr<detail::ServiceHelperBase> && helper);

    void
    insertInterface_(fhicl::ParameterSet const & pset,
                     std::unique_ptr<detail::ServiceHelperBase> && helper,
                     Factory::iterator implEntry);

    // these are real things that we use.
    art::ActivityRegistry & registry_;
    Factory factory_;
    NameIndex index_;

    TypeIDs requestedCreationOrder_;
    ServiceStack actualCreationOrder_;

  };  // ServicesManager

  // ----------------------------------------------------------------------

  template <class T,
            typename>
  T &
  ServicesManager::get()
  {
    // Find the correct Cache object.
    Factory::iterator it = factory_.find(TypeID(typeid(T)));
    if (it == factory_.end())
      throw art::Exception(art::errors::NotFound, "Service")
          << " unable to find requested service with compiler type name '"
          << cet::demangle_symbol(typeid(T).name()) << "'.\n";
    return it->second.get<T>(registry_, actualCreationOrder_);
  }  // get<>()

  template <class T,
            typename>
  T &
  ServicesManager::get(ScheduleID sID)
  {
    // Find the correct Cache object.
    Factory::iterator it = factory_.find(TypeID(typeid(T)));
    if (it == factory_.end())
      throw art::Exception(art::errors::NotFound, "Service")
          << " unable to find requested service with compiler type name '"
          << cet::demangle_symbol(typeid(T).name()) << "'.\n";
    return it->second.get<T>(registry_, actualCreationOrder_, sID);
  }  // get<>()

}  // art

// ======================================================================

#endif /* art_Framework_Services_Registry_ServicesManager_h */

// Local Variables:
// mode: c++
// End:
