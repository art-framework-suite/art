#ifndef art_Framework_IO_ProductMix_MixOp_h
#define art_Framework_IO_ProductMix_MixOp_h

// Template encapsulating all the attributes and functionality of a
// product mixing operation.

#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/IO/ProductMix/detail/checkForMissingDictionaries.h"
#include "art/Framework/IO/Root/RootBranchInfoList.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "canvas/Persistency/Provenance/BranchKey.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/InputTag.h"
#include "cetlib/exempt_ptr.h"

#include <algorithm>
#include <functional>

namespace art {
  template <typename PROD, typename OPROD>
  class MixOp;
}

template <typename PROD, typename OPROD>
class art::MixOp : public art::MixOpBase {
public:
  template <typename FUNC>
  MixOp(InputTag const& inputTag,
        std::string const& outputInstanceLabel,
        FUNC mixFunc,
        bool outputProduct,
        bool compactMissingProducts,
        BranchType bt);

  InputTag const& inputTag() const override;

  TypeID const& inputType() const override;

  std::string const& outputInstanceLabel() const override;

  void mixAndPut(Event& e, PtrRemapper const& remap) const override;

  void initializeBranchInfo(RootBranchInfoList const& rbiList) override;

  ProductID incomingProductID() const override;

  ProductID outgoingProductID() const override;

  void readFromFile(
    EntryNumberSequence const& seq,
    cet::exempt_ptr<BranchIDLists const> branchIDLists) override;

  BranchType branchType() const override;

private:
  using SpecProdList = std::vector<std::shared_ptr<Wrapper<PROD>>>;

  InputTag const inputTag_;
  TypeID const inputType_;
  std::string const outputInstanceLabel_;
  MixFunc<PROD, OPROD> const mixFunc_;
  SpecProdList inProducts_{};
  std::string const processName_;
  std::string const moduleLabel_;
  RootBranchInfo branchInfo_{};
  bool const outputProduct_;
  bool const compactMissingProducts_;
  BranchType const branchType_;
};

template <typename PROD, typename OPROD>
template <typename FUNC>
art::MixOp<PROD, OPROD>::MixOp(InputTag const& inputTag,
                               std::string const& outputInstanceLabel,
                               FUNC mixFunc,
                               bool const outputProduct,
                               bool const compactMissingProducts,
                               BranchType const bt)
  : inputTag_{inputTag}
  , inputType_{typeid(PROD)}
  , outputInstanceLabel_(outputInstanceLabel)
  , mixFunc_{mixFunc}
  , processName_{ServiceHandle<TriggerNamesService const>{}->getProcessName()}
  , moduleLabel_{ServiceHandle<CurrentModule const>{}->label()}
  , outputProduct_{outputProduct}
  , compactMissingProducts_{compactMissingProducts}
  , branchType_{bt}
{}

template <typename PROD, typename OPROD>
art::InputTag const&
art::MixOp<PROD, OPROD>::inputTag() const
{
  return inputTag_;
}

template <typename PROD, typename OPROD>
art::TypeID const&
art::MixOp<PROD, OPROD>::inputType() const
{
  return inputType_;
}

template <typename PROD, typename OPROD>
std::string const&
art::MixOp<PROD, OPROD>::outputInstanceLabel() const
{
  return outputInstanceLabel_;
}

template <typename PROD, typename OPROD>
void
art::MixOp<PROD, OPROD>::mixAndPut(Event& e, PtrRemapper const& remap) const
{
  auto rProd = std::make_unique<OPROD>();
  std::vector<PROD const*> inConverted;
  inConverted.reserve(inProducts_.size());
  try {
    auto const endIter = cend(inProducts_);
    for (auto i = cbegin(inProducts_); i != endIter; ++i) {
      auto const prod = (*i)->product();
      if (prod || !compactMissingProducts_) {
        inConverted.emplace_back(prod);
      }
    }
  }
  catch (std::bad_cast const&) {
    throw Exception(errors::DataCorruption)
      << "Unable to obtain correctly-typed product from wrapper.\n";
  }
  if (mixFunc_(inConverted, *rProd, remap)) {
    if (!outputProduct_) {
      throw Exception(errors::LogicError)
        << "Returned true (output product to be put in event) from a mix "
           "function\n"
        << "declared with outputProduct=false.\n";
    }
    if (outputInstanceLabel_.empty()) {
      e.put(std::move(rProd));
    } else {
      e.put(std::move(rProd), outputInstanceLabel_);
    }
  } // False means don't want this in the event.
}

template <typename PROD, typename OPROD>
void
art::MixOp<PROD, OPROD>::initializeBranchInfo(
  RootBranchInfoList const& branchInfo_List)
{
  if (!branchInfo_List.findBranchInfo(inputType_, inputTag_, branchInfo_)) {
    throw Exception(errors::ProductNotFound)
      << "Unable to find requested product " << inputTag_ << " of type "
      << inputType_.friendlyClassName() << " in secondary input stream.\n";
  }
  // Check dictionaries for input product, not output product: let
  // output modules take care of that.
  std::vector<TypeID> const types{TypeID{typeid(PROD)}};
  detail::checkForMissingDictionaries(types);
}

template <typename PROD, typename OPROD>
art::ProductID
art::MixOp<PROD, OPROD>::incomingProductID() const
{
  return ProductID{branchInfo_.branchName()};
}

template <typename PROD, typename OPROD>
art::ProductID
art::MixOp<PROD, OPROD>::outgoingProductID() const
{
  art::ProductID result;
  if (outputProduct_) {
    TypeID const outputType{typeid(OPROD)};
    BranchKey const key{outputType.friendlyClassName(),
                        moduleLabel_,
                        outputInstanceLabel_,
                        processName_,
                        art::InEvent}; // Outgoing product must be InEvent.
    auto I = ProductMetaData::instance().productList().find(key);
    if (I == ProductMetaData::instance().productList().end()) {
      throw Exception(errors::LogicError)
        << "MixOp unable to find branch id for a product ("
        << outputType.className() << ") that should have been registered!\n";
    }
    result = I->second.productID();
  }
  return result;
}

template <typename PROD, typename OPROD>
void
art::MixOp<PROD, OPROD>::readFromFile(
  EntryNumberSequence const& seq,
  cet::exempt_ptr<BranchIDLists const> branchIDLists)
{
  inProducts_.clear();
  inProducts_.reserve(seq.size());
  if (branchInfo_.branch() == nullptr) {
    throw Exception(errors::LogicError) << "Branch not initialized for read.\n";
  }
  configureStreamers(branchIDLists);

  // Assume the sequence is ordered per
  // MixHelper::generateEventSequence.
  auto const b = seq.cbegin(), e = seq.cend();
  for (auto i = b; i != e; ++i) {
    auto fit = std::find(b, i, *i);
    if (fit == i) { // Need new product.
      inProducts_.emplace_back(new Wrapper<PROD>);
      Wrapper<PROD>* wp = inProducts_.back().get();
      branchInfo_.branch()->SetAddress(&wp);
      branchInfo_.branch()->GetEntry(*i);
      branchInfo_.branch()->ResetAddress();
    } else { // Already have one: find and use.
      auto pit = cbegin(inProducts_);
      std::advance(pit, std::distance(b, fit));
      inProducts_.emplace_back(*pit);
    }
  }
}

template <typename PROD, typename OPROD>
inline art::BranchType
art::MixOp<PROD, OPROD>::branchType() const
{
  return branchType_;
}

#endif /* art_Framework_IO_ProductMix_MixOp_h */

// Local Variables:
// mode: c++
// End:
