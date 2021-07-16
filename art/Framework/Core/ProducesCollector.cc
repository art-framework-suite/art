#include "art/Framework/Core/ProducesCollector.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"

#include "range/v3/view/map.hpp"

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
    check_for_duplicate_friendly_names(TypeLabelLookup_t const& typeLabels)
    {
      using ranges::views::keys;
      map<string, set<string>> instanceToFriendlyNames;
      for (auto const& type_label : typeLabels | keys) {
        auto unique_entry = type_label.productInstanceName();
        if (type_label.hasEmulatedModule()) {
          // Whenever 'produces' is called, the module label is always
          // the same for a given ProducesCollector object.  However,
          // for 'reconstitutes', the user provides a module label,
          // and it is therefore necessary for this check to allow
          // different module labels with the same type and instance
          // name.
          unique_entry += "_" + type_label.emulatedModule();
        }
        auto result = instanceToFriendlyNames[unique_entry].emplace(
          type_label.typeID().friendlyClassName());
        if (!result.second) {
          throw Exception(errors::LogicError,
                          "check_for_duplicate_friendly_names: ")
            << "An attempt has been made to call the equivalent of\n\n"
            << "   produces<" << type_label.typeID().className() << ">(\""
            << type_label.productInstanceName() << "\")\n\n"
            << "which results in a prepared (\"friendly\") name of:\n\n"
            << "   " << *result.first << "\n\n"
            << "That friendly name has already been registered for this "
               "module.\n"
            << "Please check to make sure that produces<> has not already "
               "been\n"
            << "called for an Assns<> with reversed template arguments.  "
               "Such\n"
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
      check_for_duplicate_friendly_names(expectedProducts);

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
