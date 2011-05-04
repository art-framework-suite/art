#ifndef art_Framework_IO_ProductMix_MixHelper_h
#define art_Framework_IO_ProductMix_MixHelper_h

#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Framework/IO/ProductMix/MixContainerTypes.h"
#include "art/Framework/IO/ProductMix/MixOp.h"
#include "art/Framework/IO/ProductMix/ProdToProdMapBuilder.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchListIndex.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/ProductRegistry.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/TypeID.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/value_ptr.h"
#include "cpp0x/functional"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include "CLHEP/Random/RandFlat.h"

#include "boost/noncopyable.hpp"

#include "Rtypes.h"
#include "TFile.h"
#include "TTree.h"

#include <string>
#include <vector>

namespace art {
  class MixHelper;
}

class art::MixHelper : private boost::noncopyable {
public:
  MixHelper(fhicl::ParameterSet const &pset,
            ProducerBase & producesProvider);

  //////////////////////////////////////////////////////////////////////
  // produces() templates.
  //
  // Call as you would from the constructor of a module to declare
  // (e.g. bookkeeping) products to be put into the event that are *not*
  // direct results of a product mix. For the latter case, see the
  // declareMixOp() templates below.
  //////////////////////////////////////////////////////////////////////

  // Record the production of an object of type P, with optional
  // instance name, in either the Run or SubRun.
  template <class P, BranchType B>
  TypeLabel const& produces(std::string const& instanceName=std::string());

  // Record the production of an object of type P, with optional
  // instance name, in the Event.
  template <class P>
  TypeLabel const& produces(std::string const& intanceName=std::string());

  //////////////////////////////////////////////////////////////////////
  // declareMixOp templates.
  //
  // These function templates may be used by writers of product-merging
  // modules to declare a product mix operation. Such an operation may
  // be specified by providing:
  //
  // 1, an InputTag specifying which secondary products should be
  //    mixed;
  //
  // 2. an optional instance label for the mixed product; and
  //
  // 3. a mix function. This mix function should have the following
  //    signature:
  //
  //       void mixfunc(std::vector<PROD const *> const &,
  //                    PROD &,
  //                    PtrRemapper const &),
  //
  //    This function may be a free function, a function object (in
  //    which case operator() should be the member function with the
  //    correct signature) or a member function of any class provided it
  //    has the same return type and arguments.
  //
  //    For free functions, function objects and pre-bound member
  //    functions the product type template argument need not be
  //    specified as it can be deduced from the signature of the
  //    provided function.
  //
  //    If the provided function is a member function it may be provided
  //    bound to the object upon which it is to be called by the user
  //    (in which case it is treated as a free function by the
  //    registration method) or by specifying the member function
  //    followed by the object to which it should be bound (in which
  //    case the bind will be done for the user). In this latter case
  //    the template argument specifying the product type need *not* be
  //    specified usually as it may be deduced from the signature of the
  //    provided function. If one specifies an overload set however
  //    (e.g.  in the case where a class has several mix() member
  //    functions, each one with a different mix function) then the
  //    template argument must be specified in order to constrain the
  //    overload set to a single function.
  //////////////////////////////////////////////////////////////////////

  // Provide an InputTag and free function or function object.
  template <typename PROD>
  void declareMixOp(InputTag const &inputTag,
                    std::function< void (std::vector<PROD const *> const &,
                                         PROD &,
                                         PtrRemapper const &)
                    > mixFunc);


  //  Provide an InputTag, instance label, and free function or function
  //  object,
  template <typename PROD>
  void declareMixOp(InputTag const &inputTag,
                    std::string const &outputInstanceLabel,
                    std::function< void (std::vector<PROD const *> const &,
                                         PROD &,
                                         PtrRemapper const &)
                    > mixFunc);

  // Provide an InputTag, member function with the correct signature and
  // object to which the member function should be bound.
  template <typename PROD, typename T>
  void declareMixOp(InputTag const &inputTag,
                    void (T::*mixfunc) (std::vector<PROD const *> const &,
                                        PROD &,
                                        PtrRemapper const &),
                    T *t);

  // Provide an InputTag, instance label, member function with the
  // correct signature, and object to which the member function should be
  // bound.
  template <typename PROD, typename T>
  void declareMixOp(InputTag const &inputTag,
                    std::string const &outputInstanceLabel,
                    void (T::*mixfunc) (std::vector<PROD const *> const &,
                                        PROD &,
                                        PtrRemapper const &),
                    T *t);

  //////////////////////////////////////////////////////////////////////
  // Mix module writers should not need anything below this point.
  //////////////////////////////////////////////////////////////////////
  bool generateEventSequence(size_t nSecondaries,
                             EntryNumberSequence &enSeq,
                             EventIDSequence &eIDseq);
  void mixAndPut(EntryNumberSequence const &enSeq,
                 Event &e);
  void postRegistrationInit();

private:
  typedef std::vector<std::shared_ptr<MixOpBase> > MixOpList;
  typedef MixOpList::iterator MixOpIter;

  enum Mode { SEQUENTIAL, RANDOM };

  void openAndReadMetaData(std::string const &fileName);
  void buildEventIDIndex(FileIndex const &fileIndex);
  void mixAndPutOne(boost::shared_ptr<MixOpBase> mixOp,
                    EntryNumberSequence const &enSeq,
                    Event &e);
  bool openNextFile();
  void buildBranchIDTransMap(ProdToProdMapBuilder::BranchIDTransMap &transMap);

  ProducerBase &producesProvider_;
  std::vector<std::string> filenames_;
  MixOpList mixOps_;
  PtrRemapper ptrRemapper_;
  std::vector<std::string>::const_iterator currentFilename_;
  Mode readMode_;
  double coverageFraction_;
  Long64_t nEventsRead_;
  Long64_t nEventsInFile_;
  FileFormatVersion ffVersion_;
  ProdToProdMapBuilder ptpBuilder_;
  CLHEP::RandFlat dist_;

  // Root-specific state.
  EventIDIndex eventIDIndex_;
  cet::value_ptr<TFile> currentFile_;
  cet::exempt_ptr<TTree> currentMetaDataTree_;
  cet::exempt_ptr<TTree> currentEventTree_;
  RootBranchInfoList dataBranches_;
};

template <class P, art::BranchType B>
art::TypeLabel const&
art::MixHelper::produces(std::string const& instanceName) {
  return producesProvider_.produces<P, B>(instanceName);
}

template <class P>
art::TypeLabel const&
art::MixHelper::produces(std::string const& instanceName) {
  return producesProvider_.produces<P>(instanceName);
}

template <typename PROD>
void
art::MixHelper::
declareMixOp(InputTag const &inputTag,
               std::function< void (std::vector<PROD const *> const &,
                                    PROD &,
                                    PtrRemapper const &)
                            > mixFunc) {
  declareMixOp(inputTag, std::string(), mixFunc);
}

template <typename PROD>
void
art::MixHelper::
declareMixOp(InputTag const &inputTag,
               std::string const &outputInstanceLabel,
               std::function< void (std::vector<PROD const *> const &,
                                    PROD &,
                                    PtrRemapper const &)
                            > mixFunc) {
  producesProvider_.produces<PROD>(outputInstanceLabel);
  std::shared_ptr<MixOpBase> p(new MixOp<PROD>(inputTag,
                                                   outputInstanceLabel,
                                                   mixFunc));
  mixOps_.push_back(p);
}

template <typename PROD, typename T>
void
art::MixHelper::
declareMixOp(InputTag const &inputTag,
               void (T::*mixFunc) (std::vector<PROD const *> const &,
                                     PROD &,
                                     PtrRemapper const &),
               T *t) {
  declareMixOp(inputTag, std::string(), mixFunc, t);
}

template <typename PROD, typename T>
void
art::MixHelper::
declareMixOp(InputTag const &inputTag,
               std::string const &outputInstanceLabel,
               void (T::*mixFunc) (std::vector<PROD const *> const &,
                                     PROD &,
                                     PtrRemapper const &),
               T *t) {
  producesProvider_.produces<PROD>(outputInstanceLabel);
  std::shared_ptr<MixOpBase> p(new MixOp<PROD>(inputTag,
                                                   outputInstanceLabel,
                                                   std::bind(mixFunc, t,
                                                             _1, _2, _3)));
  mixOps_.push_back(p);
}

#endif /* art_Framework_IO_ProductMix_MixHelper_h */

// Local Variables:
// mode: c++
// End:
