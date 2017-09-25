#ifndef art_Framework_Principal_get_ProductDescription_h
#define art_Framework_Principal_get_ProductDescription_h

////////////////////////////////////////////////////////////////////////
// Helper class to find a product's BranchDescription in the
// MasterProductRegistry.
////////////////////////////////////////////////////////////////////////

#include "canvas/Persistency/Provenance/ProductList.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Utilities/TypeID.h"

#include <string>

namespace art {
  // Forward declarations.
  class BranchDescription;

  // 1. Get:
  // a. type info from template arg;
  // b. process name from TriggerNameService;
  // c. product registry from ProductMetaData.
  template <typename T>
  BranchDescription const&
  get_ProductDescription(BranchType branch_type,
                         std::string const& module_label,
                         std::string const& instance_name);

  // 3. Get:
  // a. type info from TypeID;
  // b. process name from TriggerNameService;
  // c. product registry from ProductMetaData.
  BranchDescription const&
  get_ProductDescription(TypeID type_id,
                         BranchType branch_type,
                         std::string const& module_label,
                         std::string const& instance_name);

  // 5. Get:
  // a. type info from TypeID;
  // b. process name from string;
  // c. product list from reference.
  BranchDescription const&
  get_ProductDescription(TypeID tid,
                         std::string const& process_name,
                         ProductDescriptionsByID const& product_list,
                         BranchType branch_type,
                         std::string const& module_label,
                         std::string const& instance_name);
}

// 1
template <typename T>
inline
art::BranchDescription const&
art::get_ProductDescription(BranchType branch_type,
                            std::string const& module_label,
                            std::string const& instance_name)
{
  return get_ProductDescription(TypeID{typeid(T)},
                                branch_type,
                                module_label,
                                instance_name); // 3.
}

#endif /* art_Framework_Principal_get_ProductDescription_h */

// Local Variables:
// mode: c++
// End:
