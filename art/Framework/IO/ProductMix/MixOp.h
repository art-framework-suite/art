#ifndef art_Framework_IO_ProductMix_MixOp_h
#define art_Framework_IO_ProductMix_MixOp_h

#include "art/Framework/Core/Event.h"
#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/IO/Root/RootBranchInfoList.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchKey.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/ConstProductRegistry.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Utilities/InputTag.h"
#include "cpp0x/functional"

namespace art {
  template <typename PROD> class MixOp;
}

template <typename PROD>
class art::MixOp : public art::MixOpBase {
public:
  template <typename FUNC>
  MixOp(InputTag const &inputTag,
          std::string const &outputInstanceLabel,
          FUNC mixFunc);

  virtual
  InputTag const &inputTag() const;

  virtual
  TypeID const &inputType() const;

  virtual
  std::string const &outputInstanceLabel() const;

  virtual
  void
  mixAndPut(Event &e,
            PtrRemapper const &remap,
            EventIDSequence const &seq) const;

  virtual
  BranchID
  incomingBranchID(RootBranchInfoList const &rbiList) const;

  virtual
  BranchID
  outgoingBranchID() const;

  virtual
  void
  readFromFile(TTree *eventTree,
               EntryNumberSequence const &seq);

private:
  typedef std::vector<Wrapper<PROD> > SpecProdList;

  void initProductList(size_t nSecondaries = 0);

  InputTag inputTag_;
  TypeID const inputType_;
  std::string outputInstanceLabel_;
  std::function<void (std::vector<PROD const *> const &, PROD &, PtrRemapper const &)> mixFunc_;
  SpecProdList inProducts_;
  typename SpecProdList::iterator prodIter_;
  typename SpecProdList::iterator productsEnd_;
  std::string processName_;
  std::string moduleLabel_;
};

template <typename PROD>
template <typename FUNC>
art::
MixOp<PROD>::MixOp(InputTag const &inputTag,
                       std::string const &outputInstanceLabel,
                       FUNC mixFunc)
  :
  inputTag_(inputTag),
  inputType_(typeid(PROD)),
  outputInstanceLabel_(outputInstanceLabel),
  mixFunc_(mixFunc),
  inProducts_(),
  prodIter_(inProducts_.begin()),
  productsEnd_(inProducts_.end()),
  processName_(ServiceHandle<TriggerNamesService>()->getProcessName()),
  moduleLabel_(ServiceHandle<CurrentModule>()->label())
{}

template <typename PROD>
art::InputTag const &
art::MixOp<PROD>::
inputTag() const {
  return inputTag_;
}

template <typename PROD>
art::TypeID const &
art::MixOp<PROD>::
inputType() const {
  return inputType_;
}

template <typename PROD>
std::string const &
art::MixOp<PROD>::
outputInstanceLabel() const {
  return outputInstanceLabel_;
}

template <typename PROD>
void
art::MixOp<PROD>::
mixAndPut(Event &e,
          PtrRemapper const &remap,
          EventIDSequence const &seq) const {
  std::auto_ptr<PROD> rProd(new PROD);
  std::vector<PROD const *> inConverted;
  inConverted.reserve(inProducts_.size());
  try {
    for (typename SpecProdList::const_iterator
           i = inProducts_.begin(),
           endIter = inProducts_.end();
         i != endIter;
         ++i) {
      inConverted.push_back(i->product());
      if (!inConverted.back()) {
        throw Exception(errors::ProductNotFound)
          << "While processing products of type "
          << TypeID(*rProd).friendlyClassName()
          << " for merging: a secondary event was missing a product.\n";
      }
    }
  }
  catch (std::bad_cast const &) {
    throw Exception(errors::DataCorruption)
      << "Unable to obtain correctly-typed product from wrapper.\n";
  }
  mixFunc_(inConverted, *rProd, remap);
  if (outputInstanceLabel_.empty()) {
    e.put(rProd);
  } else {
    e.put(rProd, outputInstanceLabel_);
  }
}

template <typename PROD>
art::BranchID
art::MixOp<PROD>::
incomingBranchID(RootBranchInfoList const &rbiList) const {
  RootBranchInfo rbi;
  if (rbiList.findBranchInfo(inputType_, inputTag_, rbi)) {
    return BranchID(rbi.branchName());
  } else {
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
outgoingBranchID() const {
  BranchKey key(inputType_.friendlyClassName(),
                moduleLabel_,
                outputInstanceLabel_,
                processName_);
  ProductRegistry const &pReg = 
    ServiceHandle<ConstProductRegistry>()->productRegistry();
  ProductRegistry::ConstProductList::const_iterator i =
    pReg.constProductList().find(key);
  if (i == pReg.constProductList().end()) {
    throw Exception(errors::LogicError)
      << "MixOp unable to find branch id for a product that should have been registered!\n";
  }
  return i->second.branchID();
}

template <typename PROD>
void
art::MixOp<PROD>::
readFromFile(TTree *eventTree,
             EntryNumberSequence const &seq) {
  initProductList(seq.size());
}

template <typename PROD>
void
art::MixOp<PROD>::
initProductList(size_t nSecondaries) {
  if (nSecondaries) {
    inProducts_.resize(nSecondaries);
  }
  prodIter_ = inProducts_.begin();
  productsEnd_ = inProducts_.end();
}

#endif /* art_Framework_IO_ProductMix_MixOp_h */

// Local Variables:
// mode: c++
// End:
