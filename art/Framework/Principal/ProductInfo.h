#ifndef art_Framework_Principal_ProductInfo_h
#define art_Framework_Principal_ProductInfo_h

#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/TypeID.h"

#include <array>
#include <set>
#include <string>
#include <tuple>
#include <vector>

namespace art {

  class ProductInfo {
  public:
    enum class ConsumableType { Product, Many, ViewElement };

    explicit ProductInfo(ConsumableType const consumableType, TypeID const& tid)
      : consumableType_{consumableType}, typeID_{tid}
    {}

    explicit ProductInfo(ConsumableType const consumableType,
                         TypeID const& tid,
                         std::string const& l,
                         std::string const& i,
                         std::string const& pr)
      : consumableType_{consumableType}
      , typeID_{tid}
      , label_{l}
      , instance_{i}
      , process_{pr}
    {}

    //    private:
    ConsumableType consumableType_;
    TypeID typeID_;
    std::string label_{};
    std::string instance_{};
    std::string process_{};
  };

  inline bool
  operator<(ProductInfo const& a, ProductInfo const& b)
  {
    return std::tie(
             a.consumableType_, a.typeID_, a.label_, a.instance_, a.process_) <
           std::tie(
             b.consumableType_, b.typeID_, b.label_, b.instance_, b.process_);
  }

  using ConsumableProductVectorPerBranch = std::vector<ProductInfo>;
  using ConsumableProductSetPerBranch = std::set<ProductInfo>;
  using ConsumableProducts =
    std::array<ConsumableProductVectorPerBranch, NumBranchTypes>;
  using ConsumableProductSets =
    std::array<ConsumableProductSetPerBranch, NumBranchTypes>;
}

#endif /* art_Framework_Principal_ProductInfo_h */

// Local Variables:
// mode: c++
// End:
