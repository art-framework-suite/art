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

    explicit ProductInfo(TypeID const& tid) :
      typeID_{tid}
    {}

    explicit ProductInfo(TypeID const& tid,
                         std::string const& l,
                         std::string const& i,
                         std::string const& pr) :
      typeID_{tid}, label_{l}, instance_{i}, process_{pr}
    {}

    bool typeOnly() const { return label_.empty() && instance_.empty() && process_.empty(); }

    //    private:
    TypeID typeID_;
    std::string label_{};
    std::string instance_{};
    std::string process_{};
  };

  inline bool operator<(ProductInfo const& a, ProductInfo const& b)
  {
    return std::tie(a.typeID_, a.label_, a.instance_, a.process_)
         < std::tie(b.typeID_, b.label_, b.instance_, b.process_);
  }

  using ConsumableProductVectorPerBranch = std::vector<ProductInfo>;
  using ConsumableProductSetPerBranch = std::set<ProductInfo>;
  using ConsumableProducts = std::array<ConsumableProductVectorPerBranch, NumBranchTypes>;
  using ConsumableProductSets = std::array<ConsumableProductSetPerBranch, NumBranchTypes>;

}


#endif /* art_Framework_Principal_ProductInfo_h */

// Local Variables:
// mode: c++
// End:
