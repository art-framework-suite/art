//#include "art/Framework/Principal/ProductInfo.h"
//// vim: set sw=2 expandtab :

//#include "canvas/Persistency/Provenance/BranchType.h"
//#include "canvas/Utilities/TypeID.h"
//
//#include <array>
//#include <set>
//#include <string>
//#include <tuple>
//#include <vector>
//
// using namespace std;
//
// namespace art {
//
//  ProductInfo(ConsumableType const consumableType, TypeID const& tid)
//    : consumableType_{consumableType}
//    , typeID_{tid}
//  {
//  }
//
//  ProductInfo(ConsumableType const consumableType, TypeID const& tid,
//  std::string const& l, std::string const& i,
//              std::string const& pr)
//    : consumableType_{consumableType}
//    , typeID_{tid}
//    , label_{l}
//    , instance_{i}
//    , process_{pr}
//  {
//  }
//
// bool
// operator<(ProductInfo const& a, ProductInfo const& b)
//{
//  return std::tie(a.consumableType_, a.typeID_, a.label_, a.instance_,
//  a.process_)
//         < std::tie(b.consumableType_, b.typeID_, b.label_, b.instance_,
//         b.process_);
//}
//
//} // namespace art
