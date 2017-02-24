#include "art/Framework/Core/get_BranchDescription.h"

#include "art/Framework/Principal/Principal.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Utilities/Exception.h"

// 3.
art::BranchDescription const&
art::get_BranchDescription(TypeID const tid,
                           BranchType const branch_type,
                           std::string const &module_label,
                           std::string const &instance_name)
{
  return get_BranchDescription(tid,
                               ServiceHandle<TriggerNamesService const>{}->getProcessName(),
                               ProductMetaData::instance().productList(),
                               branch_type,
                               module_label,
                               instance_name); // 5.
}

// 4.
art::BranchDescription const&
art::get_BranchDescription(TypeID const type_id,
                           Principal const& principal,
                           std::string const& module_label,
                           std::string const& instance_name)
{
  return get_BranchDescription(type_id,
                               principal.processConfiguration().processName(),
                               ProductMetaData::instance().productList(),
                               principal.branchType(),
                               module_label,
                               instance_name); // 5.
}

// 5.
art::BranchDescription const&
art::get_BranchDescription(TypeID const type_id,
                           std::string const& process_name,
                           ProductList const& product_list,
                           BranchType const branch_type,
                           std::string const& module_label,
                           std::string const& instance_name)
{
  BranchKey const bk {type_id.friendlyClassName(),
      module_label,
      instance_name,
      process_name,
      branch_type};
  auto const it = product_list.find(bk);
  if (it == product_list.end()) {
    throw art::Exception{art::errors::ProductRegistrationFailure, "art::get_BranchDescription"}
      << "No product is registered for\n"
      << "  process name:                '" << bk.processName_ << "'\n"
      << "  module label:                '" << bk.moduleLabel_ << "'\n"
      << "  product friendly class name: '" << bk.friendlyClassName_ << "'\n"
      << "  product instance name:       '" << bk.productInstanceName_ << "'\n"
      << "  branch type:                 '" << branch_type << "'\n";
  }
  return it->second;
}
