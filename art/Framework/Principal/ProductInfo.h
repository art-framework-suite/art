#ifndef art_Framework_Principal_ProductInfo_h
#define art_Framework_Principal_ProductInfo_h
// vim: set sw=2 expandtab :

//#include "canvas/Persistency/Provenance/BranchType.h"
//#include "canvas/Utilities/TypeID.h"
//
//#include <array>
//#include <set>
//#include <string>
//#include <tuple>
//#include <vector>
//
//namespace art {
//
//class ProductInfo {
//
//public: // TYPES
//
//  enum class ConsumableType {
//      Product // 0
//    , Many // 1
//    , ViewElement // 2
//  };
//
//public: // MEMBER FUNCTIONS -- Special Member Functions
//
//  explicit
//  ProductInfo(ConsumableType const, TypeID const&);
//
//  explicit
//  ProductInfo(ConsumableType const, TypeID const&, std::string const& label, std::string const& instance,
//              std::string const& process);
//
//public: // MEMBER DATA -- FIXME: Are these supposed to be public?
//
//  ConsumableType
//  consumableType_;
//
//  TypeID
//  typeID_;
//
//  std::string
//  label_{};
//
//  std::string
//  instance_{};
//
//  std::string
//  process_{};
//
//};
//
//bool
//operator<(ProductInfo const& a, ProductInfo const& b);
//
//using ConsumableProductVectorPerBranch = std::vector<ProductInfo>;
//using ConsumableProductSetPerBranch = std::set<ProductInfo>;
//using ConsumableProducts = std::array<ConsumableProductVectorPerBranch, NumBranchTypes>;
//using ConsumableProductSets = std::array<ConsumableProductSetPerBranch, NumBranchTypes>;
//
//} // namespace art

#endif /* art_Framework_Principal_ProductInfo_h */

// Local Variables:
// mode: c++
// End:
