#include "art/Framework/Core/ProductRegistryHelper.h"
// vim: set sw=2:

#include "art/Framework/Core/FileBlock.h"
#include "art/Persistency/Provenance/MasterProductRegistry.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Persistency/Provenance/createProductLookups.h"
#include "canvas/Persistency/Provenance/createViewLookups.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/container_algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <cassert>
#include <string>
#include <tuple>

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
  // First update the MPR with any extant products.
  mpr.updateFromModule(std::move(productList_));

  // Now go through products that will be produced in the current process.
  check_for_duplicate_Assns(typeLabelList_[InEvent]);

  ProductDescriptions descriptions;
  for (std::size_t ibt{}; ibt != NumBranchTypes; ++ibt) {
    auto const bt = static_cast<BranchType>(ibt);
    for (auto const& val : typeLabelList_[bt]) {
      BranchDescription pd{bt, val, md};
      if (val.hasEmulatedModule()) {
        // Product is provided by the source.
        presentProducts_[bt].insert(pd.productID());
      }
      else {
        // Product is produced in current process.
        producedProducts_[bt].insert(pd.productID());
      }
      descriptions.push_back(std::move(pd));
    }
  }
  productLookup_ = createProductLookups(descriptions);
  viewLookup_ = createViewLookups(descriptions);
  mpr.addProductsFromModule(move(descriptions));
}
