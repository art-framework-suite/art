#ifndef art_Framework_IO_Root_detail_KeptProvenance_h
#define art_Framework_IO_Root_detail_KeptProvenance_h

// =============================================================
// KeptProvenance: Auxiliary class to handle provenance
//                 information...so I don't lose my mind in
//                 RootOutputFile::fillBranches.

#include "art/Framework/IO/Root/DropMetaData.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/ProductProvenance.h"

#include <set>

namespace art {

  class Principal;

  namespace detail {

    class KeptProvenance {
    public:
      KeptProvenance(DropMetaData dropMetaData,
                     bool dropMetaDataForDroppedData,
                     std::set<ProductID>& branchesWithStoredHistory);

      ProductProvenance const& insert(ProductProvenance const&);
      ProductProvenance const& emplace(ProductID, ProductStatus);
      void setStatus(ProductProvenance const&, ProductStatus);

      auto begin() const { return provenance_.begin(); }
      auto end() const { return provenance_.end(); }

      void insertAncestors(ProductProvenance const& iGetParents,
                           Principal const& principal);

    private:
      bool const keepProvenance_ {true};
      DropMetaData const dropMetaData_;
      bool const dropMetaDataForDroppedData_;
      std::set<ProductID>& branchesWithStoredHistory_;
      std::set<ProductProvenance> provenance_ {};
    };

  }
}

#endif /* art_Framework_IO_Root_detail_KeptProvenance_h */

// Local variables:
// mode: c++
// End:
