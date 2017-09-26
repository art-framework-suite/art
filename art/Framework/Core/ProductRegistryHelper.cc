#include "art/Framework/Core/ProductRegistryHelper.h"
// vim: set sw=2:

#include "art/Persistency/Provenance/detail/branchNameComponentChecking.h"
#include "canvas/Persistency/Common/Assns.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/TypeLabel.h"
#include "canvas/Utilities/DebugMacros.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/TypeID.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include <cassert>
#include <memory>
#include <set>
#include <string>
#include <tuple>

using namespace std;

namespace art {

namespace {

// A user may attempt to declare:
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
void
check_for_duplicate_Assns(set<TypeLabel> const& typeLabels)
{
  map<string, set<string>> instanceToFriendlyNames;
  for (auto const& tl : typeLabels) {
    auto result = instanceToFriendlyNames[tl.productInstanceName()].emplace(tl.typeID().friendlyClassName());
    if (!result.second) {
      throw Exception(errors::LogicError, "check_for_duplicate_Assns: ")
          << "An attempt has been made to call the equivalent of\n\n"
          << "   produces<" << tl.typeID().className() << ">(\"" << tl.productInstanceName() << "\")\n\n"
          << "which results in a prepared (\"friendly\") name of:\n\n"
          << "   " << *result.first << "\n\n"
          << "That friendly name has already been registered for this module.\n"
          << "Please check to make sure that produces<> has not already been\n"
          << "called for an Assns<> with reversed template arguments.  Such\n"
          << "behavior is not supported.  Contact artists@fnal.gov for guidance.\n";
    }
  }
}

} // unnamed namespace

ProductRegistryHelper::
~ProductRegistryHelper()
{
}

ProductRegistryHelper::
ProductRegistryHelper()
{
}

void
ProductRegistryHelper::registerProducts(ProductDescriptions& producedProducts,
                                        ModuleDescription const& md)
{
  // Go through products that will be produced in the current process.
  check_for_duplicate_Assns(typeLabelList_[InEvent]);

  ProductDescriptions descriptions;
  for (std::size_t ibt{}; ibt != NumBranchTypes; ++ibt) {
    auto const bt = static_cast<BranchType>(ibt);
    for (auto const& val : typeLabelList_[bt]) {
      BranchDescription pd{bt, val, md};
      producedProducts.push_back(pd);
      descriptions.push_back(std::move(pd));
    }
  }
}


TypeLabel const&
ProductRegistryHelper::
insertOrThrow(BranchType const bt, TypeLabel const& tl)
{
  auto result = typeLabelList_[bt].insert(tl);
  return *result.first;
}

} // namespace art
