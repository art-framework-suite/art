#ifndef art_Framework_IO_Root_detail_readMetadata_h
#define art_Framework_IO_Root_detail_readMetadata_h

#include "canvas/Persistency/Provenance/rootNames.h"
#include "TBranch.h"

namespace art {
  namespace detail {
    template <typename T>
    T readMetadata(TTree* md)
    {
      auto branch = md->GetBranch(art::rootNames::metaBranchRootName<T>());
      assert(branch != nullptr);

      T mdField{};
      auto field_ptr = &mdField;
      branch->SetAddress(&field_ptr);
      input::getEntry(branch, 0);
      branch->SetAddress(nullptr);
      return mdField;
    }

    template <typename T>
    bool readMetadata(TTree* md, T& field)
    {
      auto branch = md->GetBranch(art::rootNames::metaBranchRootName<T>());
      if (branch == nullptr) {
        return false;
      }

      T mdField{};
      auto field_ptr = &mdField;
      branch->SetAddress(&field_ptr);
      input::getEntry(branch, 0);
      branch->SetAddress(nullptr);
      std::swap(mdField, field);

      return true;
    }
  }
}

#endif
