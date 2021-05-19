#include "art/Framework/Principal/ConsumesInfo.h"

#include "art/Framework/Principal/ProcessTag.h"
#include "art/Framework/Principal/ProductInfo.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/HorizontalRule.h"
#include "cetlib/container_algorithms.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <cstdlib>
#include <set>

using namespace std;

namespace art {

  ConsumesInfo::~ConsumesInfo() = default;

  ConsumesInfo::ConsumesInfo() { requireConsumes_ = false; }

  ConsumesInfo*
  ConsumesInfo::instance()
  {
    static ConsumesInfo me;
    return &me;
  }

  string
  ConsumesInfo::assemble_consumes_statement(BranchType const bt,
                                            ProductInfo const& pi)
  {
    string result;
    // Create "consumes" prefix
    switch (pi.consumableType) {
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
    result += pi.typeID.className();
    if (bt != InEvent) {
      result += ", In";
      result += BranchTypeToString(bt);
    }
    result += '>';
    // Form "(...);" string with appropriate arguments.
    result += '(';
    // Early bail out for consumesMany.
    if (pi.consumableType == ProductInfo::ConsumableType::Many) {
      result += ");";
      return result;
    }
    result += '"';
    result += pi.label;
    // If the process name is non-empty, then all InputTag fields are
    // required (e.g.):
    //   "myLabel::myProcess"
    //   "myLabel::myInstance::myProcess"
    if (!pi.process.name().empty()) {
      result += ':';
      result += pi.instance;
      result += ':';
      result += pi.process.name();
    } else if (!pi.instance.empty()) {
      result += ':';
      result += pi.instance;
    }
    result += "\");";
    return result;
  }

  string
  ConsumesInfo::module_context(ModuleDescription const& md)
  {
    string result{"module label: '"};
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
    string const& module_label,
    array<vector<ProductInfo>, NumBranchTypes> const& consumables)
  {
    std::lock_guard sentry{mutex_};
    consumables_.emplace(module_label, consumables);
  }

  void
  ConsumesInfo::validateConsumedProduct(BranchType const bt,
                                        ModuleDescription const& md,
                                        ProductInfo const& productInfo)
  {

    std::lock_guard sentry{mutex_};
    if (cet::binary_search_all(consumables_[md.moduleLabel()][bt],
                               productInfo)) {
      // Found it, everything is ok.
      return;
    }
    if (requireConsumes_.load()) {
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
    std::lock_guard sentry{mutex_};
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

} // namespace art
