#ifndef art_Framework_Core_get_BranchDescription_h
#define art_Framework_Core_get_BranchDescription_h
////////////////////////////////////////////////////////////////////////
// Helper class to find a product's BranchDescription in the principal's
// ProductRegistry.
//
// 2011/03/15 CG,
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/Principal.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ConstBranchDescription.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"

#include <string>

namespace art {
   template <typename T>
   ConstBranchDescription const &
   get_BranchDescription(T &product,
                         Principal &principal,
                         std::string const &module_label,
                         std::string const &process_name,
                         std::string const &instance_name);

}

template <typename T>
art::ConstBranchDescription const &
art::get_BranchDescription(T &product,
                           Principal &principal,
                           std::string const &module_label,
                           std::string const &process_name,
                           std::string const &instance_name) {
   TypeID typeID(typeid(T));
   BranchKey bk (typeID.friendlyClassName(),
                 module_label,
                 instance_name,
                 process_name);
   ProductRegistry::ConstProductList const& pl =
      principal.productRegistry().constProductList();
   ProductRegistry::ConstProductList::const_iterator it = pl.find(bk);
   if (it == pl.end()) {
      throw art::Exception(art::errors::InsertFailure)
         << "No product is registered for\n"
         << "  process name:                '"
         << bk.processName_ << "'\n"
         << "  module label:                '"
         << bk.moduleLabel_ << "'\n"
         << "  product friendly class name: '"
         << bk.friendlyClassName_ << "'\n"
         << "  product instance name:       '"
         << bk.productInstanceName_ << "'\n"
         << "The ProductRegistry contains:\n"
         << principal.productRegistry()
         << '\n';
   }
   if(it->second.branchType() != principal.branchType()) {
      throw art::Exception(art::errors::InsertFailure,"Not Registered")
         << "The product for ("
         << bk.friendlyClassName_ << ","
         << bk.moduleLabel_ << ","
         << bk.productInstanceName_ << ","
         << bk.processName_
         << ")\n"
         << "is registered for a(n) " << it->second.branchType()
         << " instead of for a(n) " << principal.branchType()
         << ".\n";
   }
   return it->second;
}

#endif /* art_Framework_Core_get_BranchDescription_h */

// Local Variables:
// mode: c++
// End:
