#include "art/Utilities/Globals.h"
// vim: set sw=2 expandtab :

#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <array>
#include <map>
#include <set>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

namespace art {

  ProductInfo::~ProductInfo() {}

  ProductInfo::ProductInfo(ConsumableType const consumableType,
                           TypeID const& tid)
    : consumableType_{consumableType}
    , typeID_{tid}
    , label_{}
    , instance_{}
    , process_{}
  {}

  ProductInfo::ProductInfo(ConsumableType const consumableType,
                           TypeID const& tid,
                           string const& label,
                           string const& instance,
                           string const& process)
    : consumableType_{consumableType}
    , typeID_{tid}
    , label_{label}
    , instance_{instance}
    , process_{process}
  {}

  bool
  operator<(ProductInfo const& a, ProductInfo const& b)
  {
    return tie(
             a.consumableType_, a.typeID_, a.label_, a.instance_, a.process_) <
           tie(b.consumableType_, b.typeID_, b.label_, b.instance_, b.process_);
  }

  ConsumesInfo::~ConsumesInfo() {}

  ConsumesInfo::ConsumesInfo()
    : requireConsumes_{}, consumables_{}, missingConsumes_{}
  {}

  ConsumesInfo*
  ConsumesInfo::instance()
  {
    static ConsumesInfo me;
    return &me;
  }

  std::string
  ConsumesInfo::assemble_consumes_statement(BranchType const bt,
                                            ProductInfo const& pi)
  {
    std::string result;
    // Create "consumes" prefix
    switch (pi.consumableType_) {
      case ProductInfo::ConsumableType::Product:
        result += "consumes";
        break;
      case ProductInfo::ConsumableType::ViewElement:
        result += "consumesView";
        break;
      case ProductInfo::ConsumableType::Many:
        result += "consumesMany";
        break;
    }
    // .. now time for the template arguments
    result += '<';
    result += pi.typeID_.className();
    if (bt != InEvent) {
      result += ", In";
      result += BranchTypeToString(bt);
    }
    result += '>';
    // Form "(...);" string with appropriate arguments.
    result += '(';
    // Early bail out for consumesMany.
    if (pi.consumableType_ == ProductInfo::ConsumableType::Many) {
      result += ");";
      return result;
    }
    result += '"';
    result += pi.label_;
    // If the process name is non-empty, then all InputTag fields are
    // required (e.g.):
    //   "myLabel::myProcess"
    //   "myLabel::myInstance::myProcess"
    if (!pi.process_.empty()) {
      result += ':';
      result += pi.instance_;
      result += ':';
      result += pi.process_;
    } else if (!pi.instance_.empty()) {
      result += ':';
      result += pi.instance_;
    }
    result += "\");";
    return result;
  }

  std::string
  ConsumesInfo::module_context(ModuleDescription const& md)
  {
    std::string result{"module label: '"};
    result += md.moduleLabel();
    result += "' of class type '";
    result += md.moduleName();
    result += '\'';
    return result;
  }

  void
  ConsumesInfo::setRequireConsumes(bool const val)
  {
    requireConsumes_ = val;
  }

  void
  ConsumesInfo::collectConsumes(
    std::string const& module_label,
    std::array<std::vector<ProductInfo>, NumBranchTypes> const& consumables)
  {
    consumables_.emplace(module_label, consumables);
  }

  void
  ConsumesInfo::validateConsumedProduct(BranchType const bt,
                                        ModuleDescription const& md,
                                        ProductInfo const& productInfo)
  {
    if (binary_search(consumables_[md.moduleLabel()][bt].cbegin(),
                      consumables_[md.moduleLabel()][bt].cend(),
                      productInfo)) {
      // Found it, everything is ok.
      return;
    }
    if (requireConsumes_) {
      throw Exception(errors::ProductRegistrationFailure,
                      "Consumer: an error occurred during validation of a "
                      "retrieved product\n\n")
        << "The following consumes (or mayConsume) statement is missing from\n"
        << module_context(md) << ":\n\n"
        << "  " << assemble_consumes_statement(bt, productInfo) << "\n\n";
    }
    missingConsumes_[md.moduleLabel()][bt].insert(productInfo);
  }

  void
  ConsumesInfo::showMissingConsumes() const
  {
    for (auto const& modLabelAndarySetPI : missingConsumes_) {
      auto const& modLabel = modLabelAndarySetPI.first;
      auto const& arySetPI = modLabelAndarySetPI.second;
      constexpr cet::HorizontalRule rule{60};
      mf::LogPrint log{"MTdiagnostics"};
      log << '\n'
          << rule('=') << '\n'
          << "The following consumes (or mayConsume) statements are missing "
             "from\n"
          << "module label: " << modLabel << '\n'
          << rule('-') << '\n';
      size_t i = 0;
      for (auto const& setPI : arySetPI) {
        for (auto const& pi : setPI) {
          log << "  "
              << assemble_consumes_statement(static_cast<BranchType>(i), pi)
              << '\n';
        }
        ++i;
      }
      log << rule('=');
    }
  }

  Globals::~Globals() {}

  Globals::Globals() : threads_{1}, streams_{1} {}

  Globals*
  Globals::instance()
  {
    static Globals me;
    return &me;
  }

  int
  Globals::threads()
  {
    return threads_;
  }

  int
  Globals::streams()
  {
    return streams_;
  }

  void
  Globals::setThreads(int threads)
  {
    threads_ = threads;
  }

  void
  Globals::setStreams(int streams)
  {
    streams_ = streams;
  }

} // namespace art
