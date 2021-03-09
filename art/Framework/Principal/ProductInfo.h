#ifndef art_Framework_Principal_ProductInfo_h
#define art_Framework_Principal_ProductInfo_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/ProcessTag.h"
#include "canvas/Utilities/TypeID.h"

#include <iosfwd>
#include <string>

namespace art {

  class EventProcessor;
  class Scheduler;

  class ProductInfo {
  public: // TYPES
    enum class ConsumableType { Product = 0, ViewElement = 1, Many = 2 };

    ~ProductInfo();
    explicit ProductInfo(ConsumableType, TypeID const&);
    explicit ProductInfo(ConsumableType, std::string const& friendlyName);

    explicit ProductInfo(ConsumableType,
                         TypeID const&,
                         std::string const& label,
                         std::string const& instance,
                         ProcessTag const& process);

    explicit ProductInfo(ConsumableType,
                         std::string const& friendlyName,
                         std::string const& label,
                         std::string const& instance,
                         ProcessTag const& process);

    // Future need: We need a way to tell whether consumes* or
    // mayConsume* was called.

    // Which kind of the DataViewImpl::get* functions we validate.
    ConsumableType consumableType{};

    // Data product class type.  Part 1 of branch name.  The friendly
    // class name is member is for testing reasons, where the type is
    // specified in string form.  In principle, this should be a
    // variant object instead of two separate ones.
    TypeID typeID{};
    std::string friendlyClassName{};

    // Data product module label. Part 2 of branch name.
    std::string label{};

    // Data product instance name. Part 3 of branch name.
    std::string instance{};

    // Data product process name. Part 4 of branch name.
    ProcessTag process{};
  };

  bool operator<(ProductInfo const& a, ProductInfo const& b);
  std::ostream& operator<<(std::ostream& os,
                           ProductInfo::ConsumableType const ct);
  std::ostream& operator<<(std::ostream& os, ProductInfo const& info);

} // namespace art

#endif /* art_Framework_Principal_ProductInfo_h */

// Local Variables:
// mode: c++
// End:
