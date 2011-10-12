#ifndef art_Framework_IO_ProductMix_MixHelper_h
#define art_Framework_IO_ProductMix_MixHelper_h
////////////////////////////////////////////////////////////////////////
// MixHelper
//
// Class providing, at construction time, registration services to
// users' "detail" mixing classes. (A "detail" class is the template
// argument to the instantiation of the "MixFilter" module template.)
//
//////////////////////////////////////////////////////////////////////
// declareMixOp templates.
//
// These function templates should be used by writers of product-mixing
// "detail" classes to declare each product mix operation. Such an
// operation may be specified by providing:
//
//  1. an InputTag specifying which secondary products should be mixed;
//
//  2. an optional instance label for the mixed product (defaulting to
//     the instance label of the incoming product if unspecified); and
//
//  3. a callable mixer such as:
//
//     bool mixfunc(std::vector<PROD const *> const &,
//                  PROD &,
//                  PtrRemapper const &),
//
//     As the user may prefer, the mixer may take the form of:
//       a) an arbitrarily-named free function, or
//       b) a function object whose operator() must have the correct
//          type (see below), or
//       c) an arbitrarily-named member function of any class.
//     In each case, the mixer must have the same type (i.e.,
//     the same return type and the same parameter types) illustrated
//     by "mixfunc" above. The return value of the mix function is
//     taken to indicate whether the product should be placed in the
//     event.
//
//     For free functions, function objects, and pre-bound member
//     functions, the product type template argument need not be
//     specified as it can be deduced from the signature of the provided
//     function.
//
//     If the provided function is a member function it may be provided
//     bound to the object upon which it is to be called by the user (in
//     which case it is treated as a free function by the registration
//     method) or by specifying the member function followed by the
//     object to which it should be bound (in which case the bind will
//     be done for the user). In this latter case the template argument
//     specifying the product type need *not* be specified usually as it
//     may be deduced from the signature of the provided function. If
//     one specifies an overload set however (e.g. in the case where a
//     class has several mix() member functions, each one with a
//     different mix function) then the template argument must be
//     specified in order to constrain the overload set to a single
//     function.
//
//  4. An optional boolean, "outputProduct," defaulting to, "true." A
//     false value for this parameter indicates that the mix product
//     will *never* be put into the event and should therefore not be
//     declared. If the mix operation so declared ever returns true an
//     exception will be thrown.
//
// declareMixOp() may be called with any of the following argument
// combinations:
//
//   1. Provide an InputTag and a mixer that is a free function or
//      function object.
//
//   2. Provide an InputTag, an output instance label, and a mixer that
//      is a free function or function object.
//
//   3. Provide an InputTag, a mixer that is a non-const member function
//      (of any class), and an object to which that member function should
//      be bound.
//
//   4. Provide an InputTag, an output instance label, a mixer that is a
//      non-const member function (of any class), and an object to which
//      that member function should be bound.
//
//   5. Same as 3, but providing a mixer that is a const member function.
//
//   6. Same as 4, but providing a mixer that is a const member function.
//
// Note: For signatures 3-6, if the compiler complains about an
// unresolved overload your first move should be to specify the product
// type as template argument. If that does not resolve the problem, try
// an explicit:
//
//   const_cast<T const&>(t)
//
// or
//
//   const_cast<T &>(t)
//
// as appropriate.
//
////////////////////////////////////
// produces() templates.
//
// Call as you would from the constructor of a module to declare
// (e.g., bookkeeping) products to be put into the event that are *not*
// direct results of a product mix. For the latter case, see the
// declareMixOp() templates above.
//
// Signatures for produces():
//
// 1. produces<PROD>(optional_instance_name);
//
//    Register a product to go into the event.
//
// 2. produces<PROD, art::InRun>(optional_instance_name);
//    produces<PROD, art::InSubRun>(optional_instance_name);
//
//    Register a product to go into the run or subrun.
//
////////////////////////////////////////////////////////////////////////

#include "CLHEP/Random/RandFlat.h"
#include "art/Framework/Core/ProducerBase.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Framework/IO/ProductMix/MixContainerTypes.h"
#include "art/Framework/IO/ProductMix/MixOp.h"
#include "art/Framework/IO/ProductMix/ProdToProdMapBuilder.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Persistency/Common/EDProduct.h"
#include "art/Persistency/Provenance/BranchID.h"
#include "art/Persistency/Provenance/BranchIDList.h"
#include "art/Persistency/Provenance/BranchListIndex.h"
#include "art/Persistency/Provenance/BranchType.h"
#include "art/Persistency/Provenance/FileFormatVersion.h"
#include "art/Persistency/Provenance/ProductID.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/fwd.h"
#include "boost/noncopyable.hpp"
#include "cetlib/exempt_ptr.h"
#include "cetlib/value_ptr.h"
#include "cpp0x/functional"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"

#include <string>
#include <vector>

#include "Rtypes.h"
#include "TFile.h"
#include "TTree.h"

namespace art {
  class MixHelper;
}

class art::MixHelper : private boost::noncopyable {
public:
  MixHelper(fhicl::ParameterSet const & pset,
            ProducerBase & producesProvider);

  // A.
  template <class P>
  void produces(std::string const & instanceName = std::string());

  // B.
  template <class P, BranchType B>
  void produces(std::string const & instanceName = std::string());

  // 1.
  template <typename PROD>
  void declareMixOp(InputTag const & inputTag,
                    std::function < bool (std::vector<PROD const *> const &,
                                          PROD &,
                                          PtrRemapper const &)
                    > mixFunc,
                    bool outputProduct = true);


  // 2.
  template <typename PROD>
  void declareMixOp(InputTag const & inputTag,
                    std::string const & outputInstanceLabel,
                    std::function < bool (std::vector<PROD const *> const &,
                                          PROD &,
                                          PtrRemapper const &)
                    > mixFunc,
                    bool outputProduct = true);

  // 3.
  template <typename PROD, typename T>
  void declareMixOp(InputTag const & inputTag,
                    bool (T::*mixfunc)(std::vector<PROD const *> const &,
                                       PROD &,
                                       PtrRemapper const &),
                    T & t,
                    bool outputProduct = true);

  // 4.
  template <typename PROD, typename T>
  void declareMixOp(InputTag const & inputTag,
                    std::string const & outputInstanceLabel,
                    bool (T::*mixfunc)(std::vector<PROD const *> const &,
                                       PROD &,
                                       PtrRemapper const &),
                    T & t,
                    bool outputProduct = true);

  // 5.
  template <typename PROD, typename T>
  void declareMixOp(InputTag const & inputTag,
                    bool (T::*mixfunc)(std::vector<PROD const *> const &,
                                       PROD &,
                                       PtrRemapper const &) const,
                    T const & t,
                    bool outputProduct = true);

  // 6.
  template <typename PROD, typename T>
  void declareMixOp(InputTag const & inputTag,
                    std::string const & outputInstanceLabel,
                    bool (T::*mixfunc)(std::vector<PROD const *> const &,
                                       PROD &,
                                       PtrRemapper const &) const,
                    T const & t,
                    bool outputProduct = true);

  //////////////////////////////////////////////////////////////////////
  // Mix module writers should not need anything below this point.
  //////////////////////////////////////////////////////////////////////
  bool generateEventSequence(size_t nSecondaries,
                             EntryNumberSequence & enSeq,
                             EventIDSequence & eIDseq);
  void mixAndPut(EntryNumberSequence const & enSeq,
                 Event & e);
  void postRegistrationInit();
  void setEventsToSkipFunction(std::function < size_t () > eventsToSkip);

private:
  typedef std::vector<std::shared_ptr<MixOpBase> > MixOpList;
  typedef MixOpList::iterator MixOpIter;

  enum Mode { SEQUENTIAL, RANDOM };

  void openAndReadMetaData(std::string const & fileName);
  void buildEventIDIndex(FileIndex const & fileIndex);
  void mixAndPutOne(std::shared_ptr<MixOpBase> mixOp,
                    EntryNumberSequence const & enSeq,
                    Event & e);
  bool openNextFile();
  void buildBranchIDTransMap(ProdToProdMapBuilder::BranchIDTransMap & transMap);

  ProducerBase & producesProvider_;
  std::vector<std::string> filenames_;
  MixOpList mixOps_;
  PtrRemapper ptrRemapper_;
  std::vector<std::string>::const_iterator currentFilename_;
  Mode readMode_;
  double coverageFraction_;
  Long64_t nEventsReadThisFile_;
  Long64_t nEventsInFile_;
  Long64_t totalEventsRead_;
  bool canWrapFiles_;
  FileFormatVersion ffVersion_;
  ProdToProdMapBuilder ptpBuilder_;
  CLHEP::RandFlat dist_;
  std::function < size_t () > eventsToSkip_;

  // Root-specific state.
  EventIDIndex eventIDIndex_;
  cet::value_ptr<TFile> currentFile_;
  cet::exempt_ptr<TTree> currentMetaDataTree_;
  cet::exempt_ptr<TTree> currentEventTree_;
  RootBranchInfoList dataBranches_;
};

// A.
template <class P>
inline
void
art::MixHelper::produces(std::string const & instanceName)
{
  producesProvider_.produces<P>(instanceName);
}

// B.
template <class P, art::BranchType B>
inline
void
art::MixHelper::produces(std::string const & instanceName)
{
  producesProvider_.produces<P, B>(instanceName);
}

// 1.
template <typename PROD>
inline
void
art::MixHelper::
declareMixOp(InputTag const & inputTag,
             std::function < bool (std::vector<PROD const *> const &,
                                   PROD &,
                                   PtrRemapper const &)
             > mixFunc,
             bool outputProduct)
{
  declareMixOp(inputTag, inputTag.instance(), mixFunc, outputProduct); // 2.
}

// 2.
template <typename PROD>
void
art::MixHelper::
declareMixOp(InputTag const & inputTag,
             std::string const & outputInstanceLabel,
             std::function < bool (std::vector<PROD const *> const &,
                                   PROD &,
                                   PtrRemapper const &)
             > mixFunc,
             bool outputProduct)
{
  if (outputProduct) { producesProvider_.produces<PROD>(outputInstanceLabel); }
  std::shared_ptr<MixOpBase> p(new MixOp<PROD>(inputTag,
                               outputInstanceLabel,
                               mixFunc,
                               outputProduct));
  mixOps_.push_back(p);
}

// 3.
template <typename PROD, typename T>
inline
void
art::MixHelper::
declareMixOp(InputTag const & inputTag,
             bool (T::*mixFunc)(std::vector<PROD const *> const &,
                                PROD &,
                                PtrRemapper const &),
             T & t,
             bool outputProduct)
{
  declareMixOp(inputTag, inputTag.instance(), mixFunc, t, outputProduct); // 4.
}

// 4.
template <typename PROD, typename T>
void
art::MixHelper::
declareMixOp(InputTag const & inputTag,
             std::string const & outputInstanceLabel,
             bool (T::*mixFunc)(std::vector<PROD const *> const &,
                                PROD &,
                                PtrRemapper const &),
             T & t,
             bool outputProduct)
{
  if (outputProduct) { producesProvider_.produces<PROD>(outputInstanceLabel); }
  std::shared_ptr<MixOpBase>
  p(new MixOp<PROD>(inputTag,
                    outputInstanceLabel,
                    std::bind(mixFunc, &t, _1, _2, _3),
                    outputProduct));
  mixOps_.push_back(p);
}

// 5.
template <typename PROD, typename T>
inline
void
art::MixHelper::
declareMixOp(InputTag const & inputTag,
             bool (T::*mixFunc)(std::vector<PROD const *> const &,
                                PROD &,
                                PtrRemapper const &) const,
             T const & t,
             bool outputProduct)
{
  declareMixOp(inputTag, inputTag.instance(), mixFunc, t, outputProduct); // 6.
}

// 6.
template <typename PROD, typename T>
void
art::MixHelper::
declareMixOp(InputTag const & inputTag,
             std::string const & outputInstanceLabel,
             bool (T::*mixFunc)(std::vector<PROD const *> const &,
                                PROD &,
                                PtrRemapper const &) const,
             T const & t,
             bool outputProduct)
{
  if (outputProduct) { producesProvider_.produces<PROD>(outputInstanceLabel); }
  std::shared_ptr<MixOpBase>
  p(new MixOp<PROD>(inputTag,
                    outputInstanceLabel,
                    std::bind(mixFunc, &t, _1, _2, _3),
                    outputProduct));
  mixOps_.push_back(p);
}

#endif /* art_Framework_IO_ProductMix_MixHelper_h */

// Local Variables:
// mode: c++
// End:
