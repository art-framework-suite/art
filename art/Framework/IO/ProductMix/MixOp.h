#ifndef art_Framework_IO_ProductMix_MixOp_h
#define art_Framework_IO_ProductMix_MixOp_h
// vim: set sw=2 expandtab :

// Template encapsulating all the attributes and functionality of a
// product mixing operation.

#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/IO/Root/RootBranchInfoList.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Persistency/Provenance/canonicalProductName.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/exempt_ptr.h"

#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iostream>

namespace art {

template <typename PROD, typename OPROD>
class MixOp : public MixOpBase {

public: // MEMBER FUNCTIONS -- Special Member Functions

  template <typename FUNC>
  MixOp(ModuleDescription const* md, InputTag const& inputTag, std::string const& outputInstanceLabel, FUNC mixFunc,
        bool outputProduct, bool compactMissingProducts, BranchType bt);

public: // MEMBER FUNCTIONS -- API for the user

  InputTag const&
  inputTag() const override;

  TypeID const&
  inputType() const override;

  std::string const&
  outputInstanceLabel() const override;

  void
  mixAndPut(Event& e, PtrRemapper const& remap) const override;

  void
  initializeBranchInfo(RootBranchInfoList const& rbiList) override;

  ProductID
  incomingProductID() const override;

  ProductID
  outgoingProductID() const override;

  void
  readFromFile(EntryNumberSequence const& seq, cet::exempt_ptr<BranchIDLists const> branchIDLists) override;

  BranchType
  branchType() const override;

private: // TYPES

  using SpecProdList = std::vector<std::shared_ptr<Wrapper<PROD>>>;

private: // MEMBER DATA

  InputTag const
  inputTag_;

  TypeID const
  inputType_;

  std::string const
  outputInstanceLabel_;

  MixFunc<PROD, OPROD> const
  mixFunc_;

  SpecProdList
  inProducts_{};

  std::string const
  processName_;

  ModuleDescription const*
  md_;

  RootBranchInfo
  branchInfo_{};

  bool const
  outputProduct_;

  bool const
  compactMissingProducts_;

  BranchType const
  branchType_;

};

template <typename PROD, typename OPROD>
template <typename FUNC>
MixOp<PROD, OPROD>::MixOp(ModuleDescription const* md,
                          InputTag const& inputTag,
                          std::string const& outputInstanceLabel,
                          FUNC mixFunc,
                          bool const outputProduct,
                          bool const compactMissingProducts,
                          BranchType const bt)
  : inputTag_{inputTag}
  , inputType_{typeid(PROD)}
  , outputInstanceLabel_{outputInstanceLabel}
  , mixFunc_{mixFunc}
  , processName_{ServiceHandle<TriggerNamesService const>{}->getProcessName()}
  , md_{md}
  , outputProduct_{outputProduct}
  , compactMissingProducts_{compactMissingProducts}
  , branchType_{bt}
{
}

template <typename PROD, typename OPROD>
InputTag const&
MixOp<PROD, OPROD>::inputTag() const
{
  return inputTag_;
}

template <typename PROD, typename OPROD>
TypeID const&
MixOp<PROD, OPROD>::inputType() const
{
  return inputType_;
}

template <typename PROD, typename OPROD>
std::string const&
MixOp<PROD, OPROD>::outputInstanceLabel() const
{
  return outputInstanceLabel_;
}

template <typename PROD, typename OPROD>
void
MixOp<PROD, OPROD>::mixAndPut(Event& e, PtrRemapper const& remap) const
{
  auto rProd = std::make_unique<OPROD>();
  std::vector<PROD const*> inConverted;
  inConverted.reserve(inProducts_.size());
  try {
    for (auto const& ptr : inProducts_) {
      auto const prod = ptr->product();
      if (prod || ! compactMissingProducts_) {
        inConverted.emplace_back(prod);
      }
    }
  }
  catch (std::bad_cast const&) {
    throw Exception(errors::DataCorruption)
        << "Unable to obtain correctly-typed product from wrapper.\n";
  }
  // False means don't want this in the event.
  if (mixFunc_(inConverted, *rProd, remap)) {
    if (!outputProduct_) {
      throw Exception(errors::LogicError)
          << "Returned true (output product to be put in event) from a mix function\n"
          << "declared with outputProduct=false.\n";
    }
    if (outputInstanceLabel_.empty()) {
      e.put(move(rProd));
    }
    else {
      e.put(move(rProd), outputInstanceLabel_);
    }
  }
}

template <typename PROD, typename OPROD>
void
MixOp<PROD, OPROD>::initializeBranchInfo(RootBranchInfoList const& branchInfo_List)
{
  if (!branchInfo_List.findBranchInfo(inputType_, inputTag_, branchInfo_)) {
    throw Exception(errors::ProductNotFound)
        << "Unable to find requested product "
        << inputTag_
        << " of type "
        << inputType_.friendlyClassName()
        << " in secondary input stream.\n";
  }
}

template <typename PROD, typename OPROD>
ProductID
MixOp<PROD, OPROD>::incomingProductID() const
{
  return ProductID{branchInfo_.branchName()};
}

template <typename PROD, typename OPROD>
ProductID
MixOp<PROD, OPROD>::outgoingProductID() const
{
  ProductID result;
  if (outputProduct_) {
    TypeID const outputType{typeid(OPROD)};
    // Note: Outgoing product must be InEvent.
    auto const productName = canonicalProductName(outputType.friendlyClassName(), md_->moduleLabel(), outputInstanceLabel_, processName_);
    result = ProductID{productName};
  }
  return result;
}

template <typename PROD, typename OPROD>
void
MixOp<PROD, OPROD>::readFromFile(EntryNumberSequence const& seq, cet::exempt_ptr<BranchIDLists const> branchIDLists)
{
  inProducts_.clear();
  inProducts_.reserve(seq.size());
  if (branchInfo_.branch() == nullptr) {
    throw Exception(errors::LogicError)
        << "Branch not initialized for read.\n";
  }
  configureStreamers(branchIDLists);
  // Assume the sequence is ordered per
  // MixHelper::generateEventSequence.
  auto const b = seq.cbegin(), e = seq.cend();
  for (auto i = b; i != e; ++i) {
    auto fit = std::find(b, i, *i);
    if (fit == i) {
      // Need new product.
      inProducts_.emplace_back(new Wrapper<PROD>);
      Wrapper<PROD>* wp = inProducts_.back().get();
      branchInfo_.branch()->SetAddress(&wp);
      branchInfo_.branch()->GetEntry(*i);
    }
    else {
      // Already have one: find and use.
      auto pit = inProducts_.cbegin();
      std::advance(pit, std::distance(b, fit));
      inProducts_.emplace_back(*pit);
    }
  }
}

template <typename PROD, typename OPROD>
inline
BranchType
MixOp<PROD, OPROD>::branchType() const
{
  return branchType_;
}

} // namespace art

#endif /* art_Framework_IO_ProductMix_MixOp_h */

// Local Variables:
// mode: c++
// End:
