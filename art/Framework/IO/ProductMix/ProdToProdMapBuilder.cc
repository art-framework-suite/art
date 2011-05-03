#include "art/Framework/IO/ProductMix/ProdToProdMapBuilder.h"

#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/EventPrincipal.h"
#include "art/Framework/IO/Root/rootNames.h"
#include "art/Framework/IO/Root/setMetaDataBranchAddress.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Utilities/Exception.h"

art::ProdToProdMapBuilder::ProdToProdMapBuilder()
:
  branchIDTransMap_(),
  branchIDToIndexMap_(),
  secondaryProductMap_()
{}

void
art::ProdToProdMapBuilder::prepareTranslationTables(BranchIDTransMap &transMap,
                                                    BranchIDLists const &bidl,
                                                    TTree *ehTree) {
  if (branchIDTransMap_.empty()) {
    transMap.swap(branchIDTransMap_);
  } else if (branchIDTransMap_ != transMap) {
    throw Exception(errors::DataCorruption)
      << "Secondary input file "
      << " has BranchIDs inconsistent with previous files.\n";
  }

  buildBranchIDToIndexMap(bidl);

  buildSecondaryProductMap(ehTree);
}

void
art::ProdToProdMapBuilder::populateRemapper(PtrRemapper &mapper, Event &e) const {
  mapper.setProductGetter(cet::exempt_ptr<EDProductGetter const>(e.productGetter()));

  PtrRemapper::ProdTransMap_t prodTransMap;
  
  std::transform(branchIDTransMap_.begin(),
                 branchIDTransMap_.end(),
                 std::inserter(prodTransMap, prodTransMap.begin()),
                 ProdTransMapBuilder(secondaryProductMap_,
                                     *dynamic_cast<EventPrincipal const *>
                                     (e.productGetter())));

  mapper.setProdTransMap(cet::exempt_ptr<PtrRemapper::ProdTransMap_t const>(&prodTransMap));
}

void
art::ProdToProdMapBuilder::buildBranchIDToIndexMap(BranchIDLists const &bidl) {
  BranchIDToIndexMap tmpMap;
  for (BranchIDLists::const_iterator
         bidl_beg = bidl.begin(),
         bidl_it = bidl_beg,
         bidl_end = bidl.end();
       bidl_it != bidl.end();
       ++bidl_it) {
    BranchListIndex blix = bidl_it - bidl_beg;
    for (BranchIDList::const_iterator
           bids_beg = bidl_it->begin(),
           bids_it = bidl_it->begin(),
           bids_end = bidl_it->end();
         bids_it != bids_end;
         ++bids_it) {
      ProductIndex pix = bids_it - bids_beg;
      tmpMap.insert(std::make_pair(*bids_it, std::make_pair(blix, pix)));
    }
  }
  tmpMap.swap(branchIDToIndexMap_);
}

void
art::ProdToProdMapBuilder::buildSecondaryProductMap(TTree *ehTree) {
  History h;
  History *h_p = &h;
  setMetaDataBranchAddress(ehTree, h_p);
  Long64_t nEvt = -1;
  BtoPTransMap tmpProdMap;
  while (Int_t n = ehTree->GetEntry(++nEvt)) {
    if (n > 0) {
      std::transform(branchIDTransMap_.begin(),
                     branchIDTransMap_.end(),
                     std::inserter(tmpProdMap, tmpProdMap.begin()),
                     SecondaryBranchIDToProductIDConverter(branchIDToIndexMap_, h));
    } else {
    }
    if (secondaryProductMap_.empty()) {
      tmpProdMap.swap(secondaryProductMap_);
    } else if (tmpProdMap != secondaryProductMap_) {
      throw Exception(errors::UnimplementedFeature)
        << "Unable to mix from a secondary input file which has skimmed events.\n"
        << "Contact artists@fnal.gov if you really need this feature.\n";
    }
  }
}

art::ProdToProdMapBuilder::SecondaryBranchIDToProductIDConverter::
SecondaryBranchIDToProductIDConverter(BranchIDToIndexMap const &bidi,
                                      History const &h)
  :
  bidi_(bidi),
  branchToProductIDHelper_()
{
  for(BranchListIndexes::const_iterator
        bli_beg = h.branchListIndexes().begin(),
        bli_it = bli_beg,
        bli_end = h.branchListIndexes().end();
      bli_it != bli_end;
      ++bli_it) {
    ProcessIndex pix = bli_it - bli_beg;
    branchToProductIDHelper_.insert(std::make_pair(*bli_it, pix));
  }
}

art::ProdToProdMapBuilder::SecondaryBranchIDToProductIDConverter::result_type
art::ProdToProdMapBuilder::SecondaryBranchIDToProductIDConverter::
operator()(argument_type bID) const {
  BranchIDToIndexMap::const_iterator bidi_it = bidi_.find(bID.first);
  if (bidi_it == bidi_.end()) {
    throw Exception(errors::NotFound, "Bad BranchID")
      << "Cannot determine secondary ProductID from BranchID.\n";
  }
  BLItoPIMap::const_iterator i =
    branchToProductIDHelper_.find(bidi_it->second.first);
  if (i == branchToProductIDHelper_.end()) {
    throw Exception(errors::NotFound, "Bad BranchID")
      << "Cannot determine secondary ProductID from BranchID.\n";
  }
  return std::make_pair(bID.first,
                        ProductID(i->second+1, bidi_it->second.second+1));
}

art::ProdToProdMapBuilder::ProdTransMapBuilder::
ProdTransMapBuilder(BtoPTransMap const & spMap,
                    EventPrincipal const &ep)
  :
  spMap_(spMap),
  ep_(ep)
{}

art::ProdToProdMapBuilder::ProdTransMapBuilder::result_type
art::ProdToProdMapBuilder::ProdTransMapBuilder::
operator()(argument_type bIDs) const {
  BtoPTransMap::const_iterator secondary_i = spMap_.find(bIDs.first);
  if (secondary_i == spMap_.end()) {
    throw Exception(errors::NotFound)
      << "Unable to find product translation from secondary file.\n";
  }
  return std::make_pair(secondary_i->second,
                        ep_.branchIDToProductID(bIDs.second));
}
