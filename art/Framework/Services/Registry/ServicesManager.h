#ifndef art_Framework_Services_Registry_ServicesManager_h
#define art_Framework_Services_Registry_ServicesManager_h

// ======================================================================
//
// ServicesManager
//
// ======================================================================

#include "art/Utilities/LibraryManager.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceWrapper.h"
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

/*
  We need a function in this thing that causes all the services to come to life when called.
*/

namespace art {

  typedef std::unique_ptr<art::ServiceWrapperBase> (*MAKER_t)(fhicl::ParameterSet const &, art::ActivityRegistry &);
  typedef std::unique_ptr<art::ServiceWrapperBase> (*CONVERTER_t)(std::shared_ptr<art::ServiceWrapperBase> const &);

  class ServicesManager {
    // non-copyable:
    ServicesManager(ServicesManager const &);
    void operator = (ServicesManager const &);

  public:
    typedef  std::vector<fhicl::ParameterSet>       ParameterSets;

  private:
    typedef  std::shared_ptr<ServiceWrapperBase>  WrapperBase_ptr;
    class Cache;

    typedef  std::map< TypeID, Cache >  Factory;
    typedef  std::map< std::string, Factory::iterator > NameIndex;
    typedef  std::vector< TypeID >      TypeIDs;
    typedef  std::stack< WrapperBase_ptr >  ServiceStack;

    // A Cache object encapsulates the (possibly delayed) creation of
    // a Service object.
    class Cache {
    public:
      Cache(fhicl::ParameterSet const & pset, TypeID id, MAKER_t maker, bool is_iface, Factory::iterator impl, CONVERTER_t cvt)
        : config_(pset), typeinfo_(id), make_(maker), service_(), is_interface_(is_iface), interface_impl_(impl), converter_(cvt) {
      }

      Cache(WrapperBase_ptr premade_service)
        : config_(), typeinfo_(), make_(), service_(premade_service), is_interface_(false), interface_impl_(), converter_() {
      }

      // Create the service if necessary, and return the WrapperBase_ptr
      // that refers to it.
      WrapperBase_ptr getService(ActivityRegistry & reg, ServiceStack & creationOrder) {
        if (is_interface_) {
          // This is a service interface, make the service provider do the work.
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
        else {
          if (!service_) {
            // No cached instance, we need to make it.
            createService(reg, creationOrder);
          }
        }
        return service_;
      }

      void forceCreation(ActivityRegistry & reg) {
        assert(!is_interface_ && "Cache::forceCreation called on a service interface!");
        if (!service_) {
          makeAndCacheService(reg);
        }
      }

      fhicl::ParameterSet const & getParameterSet() const {
        return config_;
      }

      void putParameterSet(fhicl::ParameterSet const & n) {
        if (config_ != n) {
          config_ = n;
          if (service_) {
            service_->reconfigure(config_);
          }
        }
      }

      Factory::iterator getInterfaceImpl() const {
        return interface_impl_;
      }

      void makeAndCacheService(ActivityRegistry & reg) {
        assert(!is_interface_ && "Cache::makeAndCacheService called on a service interface!");
        try {
          service_ = make_(config_, reg);
        }
        catch (cet::exception & e) {
          throw Exception(errors::OtherArt, "ServiceCreation", e)
              << "cet::exception caught during construction of service type "
              << cet::demangle_symbol(typeinfo_.name())
              << ":\n";
        }
        catch (std::exception & e) {
          throw Exception(errors::StdException, "ServiceCreation")
              << "std::exception caught during construction of service type "
              << cet::demangle_symbol(typeinfo_.name())
              << ": "
              << e.what();
        }
        catch (std::string const & s) {
          throw Exception(errors::BadExceptionType)
              << "String exception during construction of service type "
              << cet::demangle_symbol(typeinfo_.name())
              << ": "
              << s;
        }
        catch (...) {
          throw Exception(errors::Unknown)
              << "String exception during construction of service type "
              << cet::demangle_symbol(typeinfo_.name())
              << ":\n";
        }
      }

      void
      createService(ActivityRegistry & reg, ServiceStack & creationOrder) {
        assert(!is_interface_ && "Cache::createService called on a service interface!");
        // When we actually create the Service object, we have to
        // remember the order of creation.
        makeAndCacheService(reg);
        creationOrder.push(service_);
      }

      void
      convertService(WrapperBase_ptr & swb) {
        assert(!is_interface_ && "Cache::convertService called on a service interface!");
        swb = std::move(converter_(service_));
      }

      fhicl::ParameterSet config_;
      TypeID typeinfo_;
      MAKER_t make_;
      WrapperBase_ptr service_;
      bool is_interface_;
      Factory::iterator interface_impl_;
      CONVERTER_t converter_;

    };  // Cache

  public:
    ServicesManager(ParameterSets const &,
                    LibraryManager const &,
                    ActivityRegistry &);

    ~ServicesManager();

    template< class T >
    T &
    get();  // not const because of possible lazy creation

    //returns true of the particular service is accessible -- that is,
    // it either can be made (if requested) or has already been made.
    template< class T >
    bool
    isAvailable() const {
      return factory_.find(TypeID(typeid(T))) != factory_.end();
    }

    // TODO: needs to be converted to returning a void.
    template< class T >
    void
    put(std::shared_ptr<ServiceWrapper<T> > premade_service) {
      TypeID id(typeid(T));
      Factory::const_iterator it = factory_.find(id);
      if (it != factory_.end()) {
        throw art::Exception(art::errors::LogicError, "Service")
            << "The system has manually added service of type " << id.name()
            << ", but the service system already has a configured service of that type\n";
      }
      std::pair<Factory::iterator, bool> where = factory_.insert(std::make_pair(id, Cache(premade_service)));
      // TODO: Right now pre-made services do not have names, fix that.
      //if (service_name != "junk") {
      //  index_[service_name] = where.first;
      //}
      (void) where;
      actualCreationOrder_.push(premade_service);
    }

    // force all the services that are not alrady made into existance
    // using 'reg'.  The order of creation will be the registration order.
    void forceCreation();

    void getParameterSets(ParameterSets & out) const;
    void putParameterSets(ParameterSets const &);

  private:
    void fillFactory(ParameterSets const & psets, LibraryManager const & lm);

    // these are real things that we use.
    art::ActivityRegistry & registry_;
    Factory factory_;
    NameIndex index_;

    TypeIDs requestedCreationOrder_;
    ServiceStack actualCreationOrder_;

  };  // ServicesManager

  // ----------------------------------------------------------------------

  template< class T >
  T &
  ServicesManager::get()
  {
    // Find the correct Cache object.
    Factory::iterator it = factory_.find(TypeID(typeid(T)));
    if (it == factory_.end())
      throw art::Exception(art::errors::NotFound, "Service")
          << " unable to find requested service with compiler type name '"
          << cet::demangle_symbol(typeid(T).name()) << "'.\n";
    // Get the ServiceWrapperBase from the Cache object.
    WrapperBase_ptr swb = it->second.getService(registry_, actualCreationOrder_);
    // Cast it to the correct type.
    // Not sure this is technique is correct ... is is mostly copied
    // from the previous implementation.
    typedef ServiceWrapper<T> Wrapper;
    typedef std::shared_ptr<Wrapper> Wrapper_ptr;
    Wrapper_ptr concrete = std::dynamic_pointer_cast<Wrapper>(swb);
    return concrete->get();
  }  // get<>()

}  // art

// ======================================================================

#endif /* art_Framework_Services_Registry_ServicesManager_h */

// Local Variables:
// mode: c++
// End:
