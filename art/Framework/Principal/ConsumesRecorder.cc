#include "art/Framework/Principal/ConsumesRecorder.h"
#include "cetlib/HorizontalRule.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

namespace {
  std::string assemble_consumes_statement(art::BranchType const bt, art::ProductInfo const& pi)
  {
    std::string const& prefix = pi.typeOnly() ? "consumesMany" : "consumes";
    std::string result{prefix};
    result += '<';
    result += pi.typeID_.className();
    if (bt != art::InEvent) {
      result += ", art::In";
      result += art::BranchTypeToString(bt);
    }
    result += '>';

    // Form "(...);" string with appropriate arguments.
    result += '(';
    // Early bail out for typeOnly/consumesMany.
    if (pi.typeOnly()) {
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
}

void
art::ConsumesRecorder::setModuleDescription(ModuleDescription const& md)
{
  moduleDescription_ = &md;
}

void
art::ConsumesRecorder::prepareForJob(fhicl::ParameterSet const& pset)
{
  if (!trackConsumes_) return;

  pset.get_if_present("errorOnMissingConsumes", requireConsumes_);
  for (auto& consumablesPerBranch : consumables_) {
    cet::sort_all(consumablesPerBranch);
  }
}

void
art::ConsumesRecorder::validateConsumedProduct(BranchType const bt, ProductInfo const& pi)
{
  // Early exits if consumes tracking has been disabled or if the
  // consumed product is an allowed consumable.
  if (!trackConsumes_) return;

  if (cet::binary_search_all(consumables_[bt], pi)) return;

  if (requireConsumes_) {
    throw Exception(errors::ProductRegistrationFailure, "ConsumesRecorder: an error occurred during validation of a retrieved product\n\n")
      << "The following statement is missing from the constructor of your module:\n"
      << "  " << assemble_consumes_statement(bt, pi) << "\n\n";
  }

  missingConsumes_[bt].insert(pi);
}

void
art::ConsumesRecorder::showMissingConsumes() const
{
  if (!trackConsumes_) return;

  // If none of the branches have missing consumes statements, exit early.
  if (std::all_of(std::cbegin(missingConsumes_), std::cend(missingConsumes_),
                  [](auto const& perBranch) { return perBranch.empty();})) return;

  cet::HorizontalRule const rule{60};
  mf::LogAbsolute log{"ArtConsumes"};
  log << '\n' << rule('=') << '\n'
      << "The following consumes statements are missing from\n"
      << "  module label: '" << moduleDescription_->moduleLabel() << "' of class type '"
      << moduleDescription_->moduleName() << "'\n"
      << rule('-') << '\n';

  cet::for_all_with_index(missingConsumes_,
                          [&log](std::size_t const i, auto const& perBranch) {
                            for (auto const& pi : perBranch) {
                              log << "  " << assemble_consumes_statement(static_cast<BranchType>(i), pi) << '\n';
                            }
                          });
  log << rule('=');
}
