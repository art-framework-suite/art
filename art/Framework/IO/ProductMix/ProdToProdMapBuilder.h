#ifndef art_Framework_IO_ProductMix_ProdToProdMapBuilder_h
#define art_Framework_IO_ProductMix_ProdToProdMapBuilder_h

#include "art/Framework/Core/PtrRemapper.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchListIndex.h"
#include "art/Persistency/Provenance/History.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "cetlib/exempt_ptr.h"

#include <functional>
#include <map>

#include "Rtypes.h"
#include "TTree.h"

namespace art {
  class ProdToProdMapBuilder;
}

class art::ProdToProdMapBuilder {
public:
  typedef std::map<BranchID, BranchID> BranchIDTransMap;

  ProdToProdMapBuilder();
  void prepareTranslationTables(BranchIDTransMap & transMap,
                                BranchIDLists const & bidl,
                                TTree * ehTree);
  void populateRemapper(PtrRemapper & mapper, Event & e) const;

private:
  // Type definitions.
  typedef std::map<BranchID, std::pair<BranchListIndex, ProductIndex> >
  BranchIDToIndexMap;
  typedef std::map<BranchID, ProductID> BtoPTransMap;

  // Private member functions.
  void buildBranchIDToIndexMap(BranchIDLists const & bidl);
  void buildSecondaryProductMap(TTree * ehTree);

  // Private member data.
  BranchIDTransMap branchIDTransMap_;
  BranchIDToIndexMap branchIDToIndexMap_;
  BtoPTransMap secondaryProductMap_;

  // Nested classes.
  friend class SecondaryBranchIDToProductIDConverter;
  class SecondaryBranchIDToProductIDConverter :
    public std::unary_function < BranchIDTransMap::value_type const &,
      BtoPTransMap::value_type > {
  public:
    SecondaryBranchIDToProductIDConverter(BranchIDToIndexMap const & bidi,
                                          History const & h);
    result_type operator()(argument_type bID) const;
  private:
    typedef std::map<BranchListIndex, ProcessIndex> BLItoPIMap;
    BranchIDToIndexMap const & bidi_;
    BLItoPIMap branchToProductIDHelper_;
  };

  friend class ProdTransMapBuilder;
  class ProdTransMapBuilder :
    public std::unary_function < BranchIDTransMap::value_type const &,
      PtrRemapper::ProdTransMap_t::value_type > {
  public:
    ProdTransMapBuilder(BtoPTransMap const & spMap,
                        EventPrincipal const & ep);
    result_type operator()(argument_type bIDs) const;
  private:
    BtoPTransMap const & spMap_;
    EventPrincipal const & ep_;
  };

};
#endif /* art_Framework_IO_ProductMix_ProdToProdMapBuilder_h */

// Local Variables:
// mode: c++
// End:
