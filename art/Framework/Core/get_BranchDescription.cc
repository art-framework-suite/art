#include "art/Framework/Core/get_BranchDescription.h"

#include "art/Framework/Principal/Principal.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/BranchDescription.h"
#include "art/Utilities/Exception.h"

// 3.
art::BranchDescription const &
art::get_BranchDescription(TypeID tid,
                           BranchType branch_type,
                           std::string const & module_label,
                           std::string const & instance_name)
{
  return
    get_BranchDescription(tid,
                          ServiceHandle<TriggerNamesService>()->getProcessName(),
                          ProductMetaData::instance().productList(),
                          branch_type,
                          module_label,
                          instance_name); // 5.
}

// 4.
art::BranchDescription const &
art::get_BranchDescription(TypeID type_id,
                           Principal const & principal,
                           std::string const & module_label,
                           std::string const & instance_name)
{
  return
    get_BranchDescription(type_id,
                          principal.processConfiguration().processName(),
                          ProductMetaData::instance().productList(),
                          principal.branchType(),
                          module_label,
                          instance_name); // 5.
}

// 5.
art::BranchDescription const &
art::get_BranchDescription(TypeID type_id,
                           std::string const & process_name,
                           ProductList const & product_list,
                           BranchType branch_type,
                           std::string const & module_label,
                           std::string const & instance_name)
{
  BranchKey bk(type_id.friendlyClassName(),
               module_label,
               instance_name,
               process_name);
  ProductList::const_iterator it = product_list.find(bk);
  if (it == product_list.end()) {
    throw art::Exception(art::errors::InsertFailure)
        << "No product is registered for\n"
        << "  process name:                '"
        << bk.processName_ << "'\n"
        << "  module label:                '"
        << bk.moduleLabel_ << "'\n"
        << "  product friendly class name: '"
        << bk.friendlyClassName_ << "'\n"
        << "  product instance name:       '"
        << bk.productInstanceName_ << "'\n";
  }
  if (it->second.branchType() != branch_type) {
    throw art::Exception(art::errors::InsertFailure, "Not Registered")
        << "The product for ("
        << bk.friendlyClassName_ << ","
        << bk.moduleLabel_ << ","
        << bk.productInstanceName_ << ","
        << bk.processName_
        << ")\n"
        << "is registered for a(n) " << it->second.branchType()
        << " instead of for a(n) " << branch_type
        << ".\n";
  }
  return it->second;
}

