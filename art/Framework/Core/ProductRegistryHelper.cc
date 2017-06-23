#include "art/Framework/Core/ProductRegistryHelper.h"
// vim: set sw=2:

#include "art/Framework/Core/FileBlock.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <cassert>
#include <string>

namespace {

  // A user may wish to declare:
  //
  //   produces<Assns<A,B>>(instance_name);
  //   produces<Assns<B,A>>(instance_name);
  //
  // in their module constructor.  If the instance names are the same
  // for both produces declarations, then this is a logic error, since
  // Assns<A,B> and Assns<B,A> correspond to the same set of
  // associations.  This error can be detected by checking the
  // friendlyClassName, which resolves to the same string for
  // Assns<A,B> and Assns<B,A>.

  void check_for_duplicate_Assns(std::set<art::TypeLabel> const& typeLabels){
    std::map<std::string, std::set<std::string>> instanceToFriendlyNames;
    for (auto const& tl : typeLabels) {
      auto const& productInstanceName = tl.productInstanceName();
      auto result = instanceToFriendlyNames[productInstanceName].emplace(tl.friendlyClassName());
      if (!result.second) {
        throw art::Exception(art::errors::LogicError, "check_for_duplicate_Assns: ")
          << "An attempt has been made to call the equivalent of\n\n"
          << "   produces<" << tl.className() << ">(\"" << productInstanceName << "\")\n\n"
          << "which results in a prepared (\"friendly\") name of:\n\n"
          << "   "<< *result.first << "\n\n"
          << "That friendly name has already been registered for this module.\n"
          << "Please check to make sure that produces<> has not already been\n"
          << "called for an Assns<> with reversed template arguments.  Such\n"
          << "behavior is not supported.  Contact artists@fnal.gov for guidance.\n";
      }
    }
  }
}

void
art::ProductRegistryHelper::registerProducts(MasterProductRegistry& mpr,
                                             ModuleDescription const& md)
{
  if (productList_) {
    PerBranchTypePresence tp;
    for (auto const& val : *productList_) {
      auto const& pd = val.second;
      tp[pd.branchType()].emplace(pd.productID());
    }
    FileBlock const fb{{}, "ProductRegistryHelper"};
    mpr.initFromFirstPrimaryFile(*productList_, tp, fb);
    productList_.reset(); // Reset, since we no longer need it.
  }
  check_for_duplicate_Assns(typeLabelList_[InEvent]);
  for (std::size_t ibt{}; ibt != NumBranchTypes; ++ibt) {
    auto bt = static_cast<BranchType>(ibt);
    for (auto const& val : typeLabelList_[bt]) {
      auto pd = std::make_unique<art::BranchDescription>(bt, val, md);
      mpr.addProduct(std::move(pd));
    }
  }
}
