#ifndef art_Framework_Services_Registry_ServicesManager_h
#define art_Framework_Services_Registry_ServicesManager_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Utilities/PluginSuffixes.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/LibraryManager.h"
#include "cetlib/bold_fontify.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/fwd.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace art {

  class ActivityRegistry;
  class ProcessConfiguration;
  class ProducingServiceSignals;
  namespace detail {
    class SharedResources;
  }

  class ServicesManager {
  public:
    explicit ServicesManager(fhicl::ParameterSet&& servicesPSet,
                             ActivityRegistry& actReg,
                             detail::SharedResources& resources);
    ~ServicesManager();

    ServicesManager(ServicesManager const&) = delete;
    ServicesManager(ServicesManager&&) = delete;
    ServicesManager& operator=(ServicesManager const&) = delete;
    ServicesManager& operator=(ServicesManager&&) = delete;

    template <class T>
    bool
    isAvailable() const
    {
      return services_.find(TypeID{typeid(T)}) != cend(services_);
    }

    void getParameterSets(std::vector<fhicl::ParameterSet>& out) const;

    void forceCreation();

    // Returns vector of names corresponding to services that produce
    // products.
    std::vector<std::string> registerProducts(
      ProductDescriptions& productsToProduce,
      ProducingServiceSignals& signals,
      ProcessConfiguration const& pc);

    template <typename T>
    T& get();

    template <typename T>
    void put(std::unique_ptr<T>&& premade_service);

    template <typename SERVICE, typename... ARGS>
    void addSystemService(ARGS&&... args);

  private:
    ActivityRegistry& actReg_;
    detail::SharedResources& resources_;
    cet::LibraryManager lm_{Suffixes::service()};
    std::map<TypeID, detail::ServiceCacheEntry> services_{};
    std::vector<TypeID> requestedCreationOrder_{};
    std::stack<std::shared_ptr<detail::ServiceWrapperBase>>
      actualCreationOrder_{};
    std::vector<std::string> configErrMsgs_{};
  };

  template <typename T>
  T&
  ServicesManager::get()
  {
    auto it = services_.find(TypeID{typeid(T)});
    if (it == services_.end()) {
      throw Exception(errors::ServiceNotFound)
        << "ServicesManager unable to find the service of type '"
        << cet::demangle_symbol(typeid(T).name()) << "'.\n";
    }
    return it->second.get<T>(actReg_, resources_, actualCreationOrder_);
  }

  template <typename T>
  void
  ServicesManager::put(std::unique_ptr<T>&& premade_service)
  {
    std::unique_ptr<detail::ServiceHelperBase> service_helper(
      new detail::ServiceHelper<T>);
    TypeID const id{typeid(T)};
    auto it = services_.find(id);
    if (it != services_.end()) {
      throw Exception(errors::LogicError, "Service")
        << "The system has manually added service of type "
        << cet::demangle_symbol(id.name())
        << ", but the service system already has a configured service of that "
           "type\n";
    }
    detail::WrapperBase_ptr swb{
      new detail::ServiceWrapper<T>(std::move(premade_service))};
    actualCreationOrder_.push(swb);
    services_.emplace(
      id, detail::ServiceCacheEntry(std::move(swb), std::move(service_helper)));
  }

  template <typename SERVICE, typename... ARGS>
  void
  ServicesManager::addSystemService(ARGS&&... args) try {
    put(std::make_unique<SERVICE>(std::forward<ARGS>(args)...));
  }
  catch (fhicl::detail::validationException const& e) {
    constexpr cet::HorizontalRule rule{100};
    throw Exception(errors::Configuration)
      << "\n"
      << rule('=') << "\n\n"
      << "!! The following service has been misconfigured: !!"
      << "\n\n"
      << rule('-') << "\n\nservice_type: "
      << cet::bold_fontify(cet::demangle_symbol(typeid(SERVICE).name()))
      << "\n\n"
      << e.what() << "\n"
      << rule('=') << "\n\n";
  }

} // namespace art

#endif /* art_Framework_Services_Registry_ServicesManager_h */

// Local Variables:
// mode: c++
// End:
