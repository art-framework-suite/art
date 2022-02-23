#ifndef art_Framework_Core_ConsumesCollector_h
#define art_Framework_Core_ConsumesCollector_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ProcessTag.h"
#include "art/Framework/Principal/ProductInfo.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Utilities/TypeID.h"

#include <array>
#include <string>
#include <vector>

namespace art {
  class ConsumesCollector {
  public:
    std::array<std::vector<ProductInfo>, NumBranchTypes> const& getConsumables()
      const;
    void sortConsumables(std::string const& current_process_name);

    // Consumes information
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
    void emplace_back(BranchType,
                      ProductInfo::ConsumableType,
                      TypeID const& typeID);
    void emplace_back(BranchType,
                      ProductInfo::ConsumableType,
                      TypeID const& typeID,
                      InputTag const& tag);
    std::array<std::vector<ProductInfo>, NumBranchTypes> consumables_{};
  };

  template <typename T, BranchType BT>
  ProductToken<T>
  ConsumesCollector::consumes(InputTag const& tag)
  {
    emplace_back(
      BT, ProductInfo::ConsumableType::Product, TypeID{typeid(T)}, tag);
    return ProductToken<T>{tag};
  }

  template <typename T, BranchType BT>
  ViewToken<T>
  ConsumesCollector::consumesView(InputTag const& tag)
  {
    emplace_back(
      BT, ProductInfo::ConsumableType::ViewElement, TypeID{typeid(T)}, tag);
    return ViewToken<T>{tag};
  }

  template <typename T, BranchType BT>
  void
  ConsumesCollector::consumesMany()
  {
    emplace_back(BT, ProductInfo::ConsumableType::Many, TypeID{typeid(T)});
  }

  template <typename T, BranchType BT>
  ProductToken<T>
  ConsumesCollector::mayConsume(InputTag const& tag)
  {
    emplace_back(
      BT, ProductInfo::ConsumableType::Product, TypeID{typeid(T)}, tag);
    return ProductToken<T>{tag};
  }

  template <typename T, BranchType BT>
  ViewToken<T>
  ConsumesCollector::mayConsumeView(InputTag const& tag)
  {
    emplace_back(
      BT, ProductInfo::ConsumableType::ViewElement, TypeID{typeid(T)}, tag);
    return ViewToken<T>{tag};
  }

  template <typename T, BranchType BT>
  void
  ConsumesCollector::mayConsumeMany()
  {
    emplace_back(BT, ProductInfo::ConsumableType::Many, TypeID{typeid(T)});
  }

} // namespace art

// Local Variables:
// mode: c++
// End:

#endif /* art_Framework_Core_ConsumesCollector_h */
