#ifndef art_Framework_Services_Registry_ServicesManager_h
#define art_Framework_Services_Registry_ServicesManager_h
// vim: set sw=2 expandtab :

#include "art/Framework/Services/Registry/ServiceScope.h"
#include "art/Framework/Services/Registry/detail/ServiceCacheEntry.h"
#include "art/Framework/Services/Registry/detail/ServiceHelper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapper.h"
#include "art/Framework/Services/Registry/detail/ServiceWrapperBase.h"
#include "art/Utilities/PluginSuffixes.h"
#include "art/Utilities/bold_fontify.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/LibraryManager.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/types/detail/validationException.h"

#include <map>
#include <memory>
#include <stack>
#include <string>
#include <utility>
#include <vector>

namespace cet {
  class LibraryManager;
} // namespace cet

namespace fhicl {
  class ParameterSet;
} // namespace fhicl

namespace art {

  class ActivityRegistry;
  class ProcessConfiguration;
  class ProducingServiceSignals;

  class ServicesManager {

  public: // MEMBER FUNCTIONS -- Special Member Functions
    ~ServicesManager();

    explicit ServicesManager(fhicl::ParameterSet&& servicesPSet,
                             ActivityRegistry& actReg);

    ServicesManager(ServicesManager const&) = delete;

    ServicesManager(ServicesManager&&) = delete;

    ServicesManager& operator=(ServicesManager const&) = delete;

    ServicesManager& operator=(ServicesManager&&) = delete;

  public: // MEMBER FUNCTIONS -- Public API
    template <class T>
    bool
    isAvailable() const
    {
      return factory_.find(TypeID(typeid(T))) != factory_.end();
    }

    void getParameterSets(std::vector<fhicl::ParameterSet>& out) const;

    void forceCreation();

    void registerProducts(ProductDescriptions& productsToProduce,
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

    cet::LibraryManager lm_{Suffixes::service()};

    std::map<TypeID, detail::ServiceCacheEntry> factory_{};

    std::vector<TypeID> requestedCreationOrder_{};

    std::stack<std::shared_ptr<detail::ServiceWrapperBase>>
      actualCreationOrder_{};

    std::vector<std::string> configErrMsgs_{};
  };

  template <typename T>
  T&
  ServicesManager::get()
  {
    auto it = factory_.find(TypeID{typeid(T)});
    if (it == factory_.end()) {
      throw Exception(errors::ServiceNotFound)
        << "ServicesManager unable to find the service of type '"
        << cet::demangle_symbol(typeid(T).name()) << "'.\n";
    }
    return it->second.get<T>(actReg_, actualCreationOrder_);
  }

  template <typename T>
  void
  ServicesManager::put(std::unique_ptr<T>&& premade_service)
  {
    std::unique_ptr<detail::ServiceHelperBase> service_helper(
      new detail::ServiceHelper<T>);
    TypeID const id{typeid(T)};
    auto it = factory_.find(id);
    if (it != factory_.end()) {
      throw Exception(errors::LogicError, "Service")
        << "The system has manually added service of type "
        << cet::demangle_symbol(id.name())
        << ", but the service system already has a configured service of that "
           "type\n";
    }
    detail::WrapperBase_ptr swb{
      new detail::ServiceWrapper<T, detail::ServiceHelper<T>::scope_val>(
        std::move(premade_service))};
    actualCreationOrder_.push(swb);
    factory_.emplace(
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
      << detail::bold_fontify(cet::demangle_symbol(typeid(SERVICE).name()))
      << "\n\n"
      << e.what() << "\n"
      << rule('=') << "\n\n";
  }

} // namespace art

#endif /* art_Framework_Services_Registry_ServicesManager_h */

// Local Variables:
// mode: c++
// End:
