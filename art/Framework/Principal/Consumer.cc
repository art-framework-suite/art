#include "art/Framework/Principal/Consumer.h"
#include "cetlib/HorizontalRule.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

using namespace art;

namespace {
  std::string assemble_consumes_statement(BranchType const bt, ProductInfo const& pi)
  {
    using ConsumableType = ProductInfo::ConsumableType;
    std::string result;
    // Create "consumes" prefix
    switch(pi.consumableType_) {
    case ConsumableType::Product: result += "consumes"; break;
    case ConsumableType::Many: result += "consumesMany"; break;
    case ConsumableType::ViewElement: result += "consumesView";
    }
    // .. now time for the template arguments
    result += '<';
    result += pi.typeID_.className();
    if (bt != art::InEvent) {
      result += ", art::In";
      result += art::BranchTypeToString(bt);
    }
    result += '>';

    // Form "(...);" string with appropriate arguments.
    result += '(';
    // Early bail out for consumesMany.
    if (pi.consumableType_ == ConsumableType::Many) {
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
    }
    else if (!pi.instance_.empty()) {
      result += ':';
      result += pi.instance_;
    }
    result += "\");";
    return result;
  }

  std::string module_context(cet::exempt_ptr<ModuleDescription const> md)
  {
    std::string result{"module label: '"};
    result += (md ? md->moduleLabel() : "<invalid>");
    result += "' of class type '";
    result += (md ? md->moduleName() : "<invalid>");
    result += '\'';
    return result;
  }

}

cet::exempt_ptr<art::Consumer>
art::Consumer::non_module_context()
{
  static Consumer invalid{InvalidTag{}};
  return &invalid;
}

void
art::Consumer::setModuleDescription(ModuleDescription const& md)
{
  moduleDescription_ = &md;
}

void
art::Consumer::prepareForJob(fhicl::ParameterSet const& pset)
{
  if (!moduleContext_) return;

  pset.get_if_present("errorOnMissingConsumes", requireConsumes_);
  for (auto& consumablesPerBranch : consumables_) {
    cet::sort_all(consumablesPerBranch);
  }
}

void
art::Consumer::validateConsumedProduct(BranchType const bt, ProductInfo const& pi)
{
  // Early exits if consumes tracking has been disabled or if the
  // consumed product is an allowed consumable.
  if (!moduleContext_) return;

  if (cet::binary_search_all(consumables_[bt], pi)) return;

  if (requireConsumes_) {
    throw Exception(errors::ProductRegistrationFailure, "Consumer: an error occurred during validation of a retrieved product\n\n")
      << "The following consumes (or mayConsume) statement is missing from\n"
      << module_context(moduleDescription_) << ":\n\n"
      << "  " << assemble_consumes_statement(bt, pi) << "\n\n";
  }

  missingConsumes_[bt].insert(pi);
}

void
art::Consumer::showMissingConsumes() const
{
  if (!moduleContext_) return;

  // If none of the branches have missing consumes statements, exit early.
  if (std::all_of(cbegin(missingConsumes_), cend(missingConsumes_),
                  [](auto const& perBranch) { return perBranch.empty();})) return;

  constexpr cet::HorizontalRule rule{60};
  mf::LogPrint log{"MTdiagnostics"};
  log << '\n' << rule('=') << '\n'
      << "The following consumes (or mayConsume) statements are missing from\n"
      << module_context(moduleDescription_) << '\n'
      << rule('-') << '\n';

  cet::for_all_with_index(missingConsumes_,
                          [&log](std::size_t const i, auto const& perBranch) {
                            for (auto const& pi : perBranch) {
                              log << "  " << assemble_consumes_statement(static_cast<BranchType>(i), pi) << '\n';
                            }
                          });
  log << rule('=');
}
