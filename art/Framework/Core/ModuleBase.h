#ifndef art_Framework_Core_ModuleBase_h
#define art_Framework_Core_ModuleBase_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/ConsumesCollector.h"
#include "art/Framework/Principal/ProductInfo.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductToken.h"

#include <array>
#include <optional>
#include <string>
#include <vector>

namespace art {
  class ModuleBase {
  public:
    virtual ~ModuleBase() noexcept;
    ModuleBase();

    ModuleDescription const& moduleDescription() const;
    void setModuleDescription(ModuleDescription const&);

    std::array<std::vector<ProductInfo>, NumBranchTypes> const& getConsumables()
      const;
    void sortConsumables(std::string const& current_process_name);

  protected:
    // Consumes information
    ConsumesCollector& consumesCollector();

    template <typename T, BranchType = InEvent>
    ProductToken<T> consumes(InputTag const&);
    template <typename Element, BranchType = InEvent>
    ViewToken<Element> consumesView(InputTag const&);
    template <typename T, BranchType = InEvent>
    void consumesMany();

    template <typename T, BranchType = InEvent>
    ProductToken<T> mayConsume(InputTag const&);
    template <typename Element, BranchType = InEvent>
    ViewToken<Element> mayConsumeView(InputTag const&);
    template <typename T, BranchType = InEvent>
    void mayConsumeMany();

  private:
    std::optional<ModuleDescription> md_{std::nullopt};
    ConsumesCollector collector_;
  };

  template <typename T, BranchType BT>
  ProductToken<T>
  ModuleBase::consumes(InputTag const& tag)
  {
    return collector_.consumes<T, BT>(tag);
  }

  template <typename T, BranchType BT>
  ViewToken<T>
  ModuleBase::consumesView(InputTag const& tag)
  {
    return collector_.consumesView<T, BT>(tag);
  }

  template <typename T, BranchType BT>
  void
  ModuleBase::consumesMany()
  {
    collector_.consumesMany<T, BT>();
  }

  template <typename T, BranchType BT>
  ProductToken<T>
  ModuleBase::mayConsume(InputTag const& tag)
  {
    return collector_.mayConsume<T, BT>(tag);
  }

  template <typename T, BranchType BT>
  ViewToken<T>
  ModuleBase::mayConsumeView(InputTag const& tag)
  {
    return collector_.mayConsumeView<T, BT>(tag);
  }

  template <typename T, BranchType BT>
  void
  ModuleBase::mayConsumeMany()
  {
    collector_.mayConsumeMany<T, BT>();
  }

} // namespace art

#endif /* art_Framework_Core_ModuleBase_h */

// Local Variables:
// mode: c++
// End:
