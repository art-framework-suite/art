#include "art/Framework/Core/ProducesCollector.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"

#include <map>
#include <set>
#include <string>

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
    check_for_duplicate_Assns(TypeLabelLookup_t const& typeLabels)
    {
      map<string, set<string>> instanceToFriendlyNames;
      for (auto const& pr : typeLabels) {
        auto const& tl = pr.first;
        auto result = instanceToFriendlyNames[tl.productInstanceName()].emplace(
          tl.typeID().friendlyClassName());
        if (!result.second) {
          throw Exception(errors::LogicError, "check_for_duplicate_Assns: ")
            << "An attempt has been made to call the equivalent of\n\n"
            << "   produces<" << tl.typeID().className() << ">(\""
            << tl.productInstanceName() << "\")\n\n"
            << "which results in a prepared (\"friendly\") name of:\n\n"
            << "   " << *result.first << "\n\n"
            << "That friendly name has already been registered for this "
               "module.\n"
            << "Please check to make sure that produces<> has not already "
               "been\n"
            << "called for an Assns<> with reversed template arguments.  Such\n"
            << "behavior is not supported.  Contact artists@fnal.gov for "
               "guidance.\n";
        }
      }
    }

  } // unnamed namespace

  TypeLabel const&
  ProducesCollector::insertOrThrow(BranchType const bt, TypeLabel const& tl)
  {
    auto result = typeLabelList_[bt].emplace(tl, BranchDescription{});
    if (!result.second) {
      ostringstream oss;
      oss << "An attempt was made to declare the following product in the same "
             "module:\n"
          << "Branch type: " << bt << '\n'
          << "Class name: " << tl.className() << '\n'
          << "Product instance name: " << tl.productInstanceName() << '\n';
      throw Exception{
        errors::ProductRegistrationFailure,
        "An error occurred during a call to 'produces' or 'reconstitutes'."}
        << oss.str();
    }
    return result.first->first;
  }

  void
  ProducesCollector::fillDescriptions(ModuleDescription const& md)
  {
    auto fillDescriptionsPerBT = [this, &md](BranchType const bt) {
      auto& expectedProducts = typeLabelList_[bt];
      // Go through products that will be produced in the current process.
      check_for_duplicate_Assns(expectedProducts);

      for (auto& [typeLabel, pd] : expectedProducts) {
        pd = BranchDescription{bt,
                               typeLabel,
                               md.moduleLabel(),
                               md.parameterSetID(),
                               md.processConfiguration()};
      }
    };
    for_each_branch_type(fillDescriptionsPerBT);
  }

}
