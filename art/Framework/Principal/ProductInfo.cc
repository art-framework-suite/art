#include "art/Framework/Principal/ProductInfo.h"

#include <tuple>

using namespace std;

namespace art {

  ProductInfo::~ProductInfo() = default;

  ProductInfo::ProductInfo(ConsumableType const consumableType,
                           TypeID const& tid)
    : consumableType{consumableType}
    , typeID{tid}
    , friendlyClassName{typeID.friendlyClassName()}
  {}

  ProductInfo::ProductInfo(ConsumableType const consumableType,
                           std::string const& friendlyName)
    : consumableType{consumableType}, friendlyClassName{friendlyName}
  {}

  ProductInfo::ProductInfo(ConsumableType const consumableType,
                           TypeID const& tid,
                           string const& label,
                           string const& instance,
                           ProcessTag const& process)
    : consumableType{consumableType}
    , typeID{tid}
    , friendlyClassName{typeID.friendlyClassName()}
    , label{label}
    , instance{instance}
    , process{process}
  {}

  ProductInfo::ProductInfo(ConsumableType const consumableType,
                           string const& friendlyClassName,
                           string const& label,
                           string const& instance,
                           ProcessTag const& process)
    : consumableType{consumableType}
    , friendlyClassName{friendlyClassName}
    , label{label}
    , instance{instance}
    , process{process}
  {}

  bool
  operator<(ProductInfo const& a, ProductInfo const& b)
  {
    auto const& boundA = tie(a.consumableType,
                             a.friendlyClassName,
                             a.label,
                             a.instance,
                             a.process.name());
    auto const& boundB = tie(b.consumableType,
                             b.friendlyClassName,
                             b.label,
                             b.instance,
                             b.process.name());
    return boundA < boundB;
  }

  ostream&
  operator<<(ostream& os, ProductInfo::ConsumableType const ct)
  {
    switch (ct) {
      case ProductInfo::ConsumableType::Product:
        os << "Product";
        break;
      case ProductInfo::ConsumableType::ViewElement:
        os << "ViewElement";
        break;
      case ProductInfo::ConsumableType::Many:
        os << "Many";
        break;
    }
    return os;
  }

  ostream&
  operator<<(ostream& os, ProductInfo const& info)
  {
    os << "Consumable type: " << info.consumableType << '\n'
       << "Friendly class name: " << info.friendlyClassName << '\n'
       << "Module label: " << info.label << '\n'
       << "Instance name: " << info.instance << '\n'
       << "Process name: " << info.process.name() << '\n';
    return os;
  }

} // namespace art
