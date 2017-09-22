#include "art/Framework/Principal/get_ProductDescription.h"

#include "art/Framework/Principal/Principal.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchDescription.h"
#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/canonicalProductName.h"
#include "canvas/Utilities/Exception.h"

// 3.
art::BranchDescription const&
art::get_ProductDescription(TypeID const tid,
                            BranchType const branch_type,
                            std::string const& module_label,
                            std::string const& instance_name)
{
  return get_ProductDescription(tid,
                                ServiceHandle<TriggerNamesService const>{}->getProcessName(),
                                ProductMetaData::instance().productLists()[branch_type],
                                branch_type,
                                module_label,
                                instance_name); // 5.
}

// 4.
art::BranchDescription const&
art::get_ProductDescription(TypeID const type_id,
                            Principal const& principal,
                            std::string const& module_label,
                            std::string const& instance_name)
{
  return get_ProductDescription(type_id,
                                principal.processConfiguration().processName(),
                                ProductMetaData::instance().productLists()[principal.branchType()],
                                principal.branchType(),
                                module_label,
                                instance_name); // 5.
}

// 5.
art::BranchDescription const&
art::get_ProductDescription(TypeID const type_id,
                            std::string const& process_name,
                            ProductDescriptionsByID const& descriptions,
                            BranchType const branch_type,
                            std::string const& module_label,
                            std::string const& instance_name)
{
  auto const& product_name = canonicalProductName(type_id.friendlyClassName(),
                                                  module_label,
                                                  instance_name,
                                                  process_name);
  ProductID const pid{product_name};
  auto const it = descriptions.find(pid);
  if (it == descriptions.end()) {
    throw art::Exception{art::errors::ProductRegistrationFailure, "art::get_ProductDescription"}
    << "No product is registered for\n"
         << "  process name:                '" << process_name << "'\n"
         << "  module label:                '" << module_label << "'\n"
         << "  product friendly class name: '" << type_id.friendlyClassName() << "'\n"
         << "  product instance name:       '" << instance_name << "'\n"
         << "  branch type:                 '" << branch_type << "'\n";
  }
  return it->second;
}
