#ifndef art_Framework_IO_Sources_put_product_in_principal_h
#define art_Framework_IO_Sources_put_product_in_principal_h
////////////////////////////////////////////////////////////////////////
// Helper class to put products directly into the {Run, SubRun, Event}
// Principal.
//
// NOTE that this should *not* be used to put products in the event that
// require parentage informtion to be filled in.
//
// 2011/03/15 CG,
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/get_BranchDescription.h"
#include "art/Framework/Core/detail/maybe_call_post_insert.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ConstBranchDescription.h"
#include "art/Persistency/Provenance/ProductProvenance.h"
#include "art/Persistency/Provenance/ProductStatus.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"

#include <memory>
#include <string>

namespace art {
   template <typename T, typename P>
   void
   put_product_in_principal(std::auto_ptr<T> product,
                            P &principal,
                            std::string const &module_label,
                            std::string const &instance_name = std::string());

}

template <typename T, typename P>
void
art::put_product_in_principal(std::auto_ptr<T> product,
                              P &principal,
                              std::string const &module_label,
                              std::string const &instance_name) {
   if (product.get() == 0) {
      TypeID typeID(typeid(T));
      throw art::Exception(art::errors::NullPointerError)
         << "put_product_in_principal: A null auto_ptr was passed to 'put'.\n"
         << "The pointer is of type " << typeID << ".\n"
         << "The specified product instance name was '"
         << instance_name << "'.\n";
   }

   detail::maybe_call_post_insert(product.get());

   ConstBranchDescription const &desc =
      get_BranchDescription(*product,
                            principal,
                            module_label,
                            instance_name);

   std::auto_ptr<EDProduct> wp(new Wrapper<T>(product));
   std::auto_ptr<ProductProvenance>
      runEntryInfoPtr(new ProductProvenance(desc.branchID(),
                                            productstatus::present()));
   principal.put(wp, desc, runEntryInfoPtr);
}

#endif /* art_Framework_IO_Sources_put_product_in_principal_h */

// Local Variables:
// mode: c++
// End:
