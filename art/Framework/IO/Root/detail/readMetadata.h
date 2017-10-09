#ifndef art_Framework_IO_Root_detail_readMetadata_h
#define art_Framework_IO_Root_detail_readMetadata_h

#include "TBranch.h"
#include "art/Framework/IO/Root/detail/getObjectRequireDict.h"
#include "canvas/Persistency/Provenance/rootNames.h"
#include "canvas/Utilities/TypeID.h"

namespace art {
  namespace detail {
    template <typename T>
    T
    readMetadata(TTree* md, bool const requireDict = true)
    {
      auto branch = md->GetBranch(art::rootNames::metaBranchRootName<T>());
      assert(branch != nullptr);

      auto mdField = requireDict ? root::getObjectRequireDict<T>() : T{};
      auto field_ptr = &mdField;
      branch->SetAddress(&field_ptr);
      input::getEntry(branch, 0);
      branch->SetAddress(nullptr);
      return mdField;
    }

    template <typename T>
    bool
    readMetadata(TTree* md, T& field, bool const requireDict = true)
    {
      auto branch = md->GetBranch(art::rootNames::metaBranchRootName<T>());
      if (branch == nullptr) {
        return false;
      }

      auto mdField = requireDict ? root::getObjectRequireDict<T>() : T{};
      auto field_ptr = &mdField;
      branch->SetAddress(&field_ptr);
      input::getEntry(branch, 0);
      branch->SetAddress(nullptr);
      std::swap(mdField, field);

      return true;
    }
  }
}

#endif /* art_Framework_IO_Root_detail_readMetadata_h */

// Local Variables:
// mode: c++
// End:
