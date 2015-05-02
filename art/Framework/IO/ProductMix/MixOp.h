#ifndef art_Framework_IO_ProductMix_MixOp_h
#define art_Framework_IO_ProductMix_MixOp_h

// Template encapsulating all the attributes and functionality of a
// product mixing operation.

#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/IO/Root/RootBranchInfoList.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Framework/IO/Root/RefCoreStreamer.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ProductList.h"
#include "art/Persistency/Provenance/ProductMetaData.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/InputTag.h"
#include "cpp0x/functional"

#include "TBranch.h"

namespace art {
  template <typename PROD> class MixOp;
}

template <typename PROD>
class art::MixOp : public art::MixOpBase {
public:
  template <typename FUNC>
  MixOp(InputTag const & inputTag,
        std::string const & outputInstanceLabel,
        FUNC mixFunc,
        bool outputProduct,
        bool compactMissingProducts);

  virtual
  InputTag const & inputTag() const;

  virtual
  TypeID const & inputType() const;

  virtual
  std::string const & outputInstanceLabel() const;

  virtual
  void
  mixAndPut(Event & e,
            PtrRemapper const & remap) const;

  virtual
  void
  initializeBranchInfo(RootBranchInfoList const & rbiList);

  virtual
  BranchID
  incomingBranchID() const;

  virtual
  BranchID
  outgoingBranchID() const;

  virtual
  void
  readFromFile(EntryNumberSequence const & seq);

private:
  typedef std::vector<Wrapper<PROD> > SpecProdList;

  void initProductList(size_t nSecondaries = 0);

  InputTag const inputTag_;
  TypeID const inputType_;
  std::string const outputInstanceLabel_;
  std::function<bool (std::vector<PROD const *> const &, PROD &, PtrRemapper const &)> const mixFunc_;
  SpecProdList inProducts_;
  std::string const processName_;
  std::string const moduleLabel_;
  RootBranchInfo branchInfo_;
  bool const outputProduct_;
  bool const compactMissingProducts_;
};

template <typename PROD>
template <typename FUNC>
art::
MixOp<PROD>::MixOp(InputTag const & inputTag,
                   std::string const & outputInstanceLabel,
                   FUNC mixFunc,
                   bool outputProduct,
                   bool compactMissingProducts)
  :
  inputTag_(inputTag),
  inputType_(typeid(PROD)),
  outputInstanceLabel_(outputInstanceLabel),
  mixFunc_(mixFunc),
  inProducts_(),
  processName_(ServiceHandle<TriggerNamesService>()->getProcessName()),
  moduleLabel_(ServiceHandle<CurrentModule>()->label()),
  branchInfo_(),
  outputProduct_(outputProduct),
  compactMissingProducts_(compactMissingProducts)
{}

template <typename PROD>
art::InputTag const &
art::MixOp<PROD>::
inputTag() const
{
  return inputTag_;
}

template <typename PROD>
art::TypeID const &
art::MixOp<PROD>::
inputType() const
{
  return inputType_;
}

template <typename PROD>
std::string const &
art::MixOp<PROD>::
outputInstanceLabel() const
{
  return outputInstanceLabel_;
}

template <typename PROD>
void
art::MixOp<PROD>::
mixAndPut(Event & e,
          PtrRemapper const & remap) const
{
  std::unique_ptr<PROD> rProd(new PROD()); // Parens necessary for native types.
  std::vector<PROD const *> inConverted;
  inConverted.reserve(inProducts_.size());
  try {
    for (typename SpecProdList::const_iterator
         i = inProducts_.begin(),
         endIter = inProducts_.end();
         i != endIter;
         ++i) {
      auto prod = i->product();
      if (prod || ! compactMissingProducts_) {
        inConverted.emplace_back(prod);
      }
    }
  }
  catch (std::bad_cast const &) {
    throw Exception(errors::DataCorruption)
        << "Unable to obtain correctly-typed product from wrapper.\n";
  }
  if (mixFunc_(inConverted, *rProd, remap)) {
    if (!outputProduct_) {
      throw Exception(errors::LogicError)
          << "Returned true (output product to be put in event) from a mix function\n"
          << "declared with outputProduct=false.\n";
    }
    if (outputInstanceLabel_.empty()) {
      e.put(std::move(rProd));
    }
    else {
      e.put(std::move(rProd), outputInstanceLabel_);
    }
  } // false means don't want this in the event.
}

template <typename PROD>
void
art::MixOp<PROD>::
initializeBranchInfo(RootBranchInfoList const & branchInfo_List)
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

template <typename PROD>
art::BranchID
art::MixOp<PROD>::
incomingBranchID() const
{
  return BranchID(branchInfo_.branchName());
}

template <typename PROD>
art::BranchID
art::MixOp<PROD>::
outgoingBranchID() const
{
  art::BranchID result;
  if (outputProduct_) {
    BranchKey key(inputType_.friendlyClassName(),
                  moduleLabel_,
                  outputInstanceLabel_,
                  processName_);
    auto I = ProductMetaData::instance().productList().find(key);
    if (I == ProductMetaData::instance().productList().end()) {
      throw Exception(errors::LogicError)
          << "MixOp unable to find branch id for a product that "
             "should have been registered!\n";
    }
    result = I->second.branchID();
  }
  return result;
}

template <typename PROD>
void
art::MixOp<PROD>::
readFromFile(EntryNumberSequence const & seq)
{
  if (branchInfo_.branch() == 0) {
    throw Exception(errors::LogicError)
        << "Branch not initialized for read.\n";
  }
  initProductList(seq.size());
  // Make sure we don't have a ProductGetter set.
  configureRefCoreStreamer();
  // Assume the sequence is ordered per
  // MixHelper::generateEventSequence.
  typename SpecProdList::iterator prod_iter =
    inProducts_.begin();
  for (EntryNumberSequence::const_iterator
       i = seq.begin(),
       e = seq.end();
       i != e;
       ++i) {
    typename SpecProdList::value_type * wp = &(*prod_iter++);
    branchInfo_.branch()->SetAddress(&wp);
    branchInfo_.branch()->GetEntry(*i);
  }
}

template <typename PROD>
void
art::MixOp<PROD>::
initProductList(size_t nSecondaries) {
  inProducts_.resize(nSecondaries);
}

#endif /* art_Framework_IO_ProductMix_MixOp_h */

// Local Variables:
// mode: c++
// End:
