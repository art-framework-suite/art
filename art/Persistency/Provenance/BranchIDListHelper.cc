#include "art/Persistency/Provenance/BranchIDListHelper.h"
#include "art/Persistency/Provenance/BranchIDListRegistry.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Utilities/EDMException.h"

namespace art {

  void
  BranchIDListHelper:: updateFromInput(BranchIDLists const& bidlists, std::string const& fileName) {
    typedef BranchIDListRegistry::const_iterator iter;
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    BranchIDListRegistry::collection_type& bdata = breg.data();
    iter j = bidlists.begin(), jEnd = bidlists.end();
    for(iter i = bdata.begin(), iEnd = bdata.end(); j != jEnd && i != iEnd; ++j, ++i) {
      if (*i != *j) {
	throw art::Exception(errors::UnimplementedFeature)
	  << "Cannot merge file '" << fileName << "' due to a branch mismatch.\n"
	  << "Contact the framework group.\n";
      }
    }
    for (; j != jEnd; ++j) {
      breg.insertMapped(*j);
    }
  }

  void
  BranchIDListHelper::updateRegistries(ProductRegistry const& preg) {
    BranchIDList bidlist;
    // Add entries for current process for ProductID to BranchID mapping.
    for (ProductRegistry::ProductList::const_iterator it = preg.productList().begin(), itEnd = preg.productList().end();
        it != itEnd; ++it) {
      if (it->second.produced()) {
        if (it->second.branchType() == InEvent) {
          bidlist.push_back(it->second.branchID().id());
        }
      }
    }
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    breg.insertMapped(bidlist);

    // Add entries to aid BranchID to ProductID mapping
    BranchIDToIndexMap& branchIDToIndexMap = breg.extra().branchIDToIndexMap_;
    for (BranchIDLists::const_iterator it = breg.data().begin(), itEnd = breg.data().end(); it != itEnd; ++it) {
      BranchListIndex blix = it - breg.data().begin();
      for (BranchIDList::const_iterator i = it->begin(), iEnd = it->end(); i != iEnd; ++i) {
        ProductIndex pix = i - it->begin();
	branchIDToIndexMap.insert(std::make_pair(*i, std::make_pair(blix, pix)));
      }
    }
  }

  void
  BranchIDListHelper::clearRegistries() {
    BranchIDListRegistry& breg = *BranchIDListRegistry::instance();
    breg.data().clear();
    breg.extra().branchIDToIndexMap_.clear();
  }
}
