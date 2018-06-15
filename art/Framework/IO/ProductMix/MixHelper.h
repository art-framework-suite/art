#ifndef art_Framework_IO_ProductMix_MixHelper_h
#define art_Framework_IO_ProductMix_MixHelper_h
////////////////////////////////////////////////////////////////////////
// MixHelper
//
// Class providing, at construction time, registration services to
// users' "detail" mixing classes. (A "detail" class is the template
// argument to the instantiation of the "MixFilter" module template.)
//
////////////////////////////////////////////////////////////////////////
// Configuration.
//
// MixHelper will be passed the configuration of the module. The
// following items are significant:
//
// fileNames (default empty)
//
//   Sequence of secondary files for mixing. However, see the function
//   registerSecondaryFileNameProvider(...) below. If a secondary file
//   name provider is *not* registered, it is an error to have an
//   empty fileNames.
//
// readMode (default sequential).
//
//   Specify how events should be chosen from each file. Valid values
//   are:
//
//     sequential -- read the secondary events in order
//     randomReplace -- random with replacement
//     randomLimReplace -- events unique within a primary event
//     randomNoReplace -- events guaranteed to be used once only.
//
// MT note: What gets mixed is unpredictable when multiple mixing
//          modules run in parallel either because of multiple trigger
//          paths or because of streams when the mixing mode is
//          Mode::SEQUENTIAL.  For art 3.0, mixing will be entirely
//          serialized.
//
// coverageFraction (default 1.0).
//
//   Ratio of sampled to total events in a file. Used by randomReplace
//   and randomLimReplace modes only.
//
// wrapFiles (default false).
//
//   Re-start from fileNames[0] after secondary events are
//   exhausted. If this is false, exhausting the secondary file stream
//   will result in the filter returning false for the remainder of
//   the job.
//
// compactMissingProducts (default false).
//
//   In the case of a secondary event missing a particular product,
//   the sequence of product pointers passed to the MixOp will be
//   compacted to remove nullptrs.
//
////////////////////////////////////////////////////////////////////////
// readMode()
//
// Return the enumerated value representing the event mixing strategy.
//
////////////////////////////////////////////////////////////////////////
// registerSecondaryFileNameProvider(<function> func)
//
// Register the provided function as a provider of file names for
// mixing. This should be called from the constructor of your detail
// object.
//
// <function> must be convertible to std::function<std::string()>. A
// free function taking no arguments and returning std::string, a
// functor whose operator() has the same signature, or a bound free or
// member function whose signature after binding is std::string() are
// all convertible to std::function<std::string()>.
//
// E.g. for a detail class with member function std::string getMixFile():
//
//  registerSecondaryFileNameProvider(std::bind(&Detail::getMixFile, this));
//
// Notes:
//
// 1. It is a configuration error to provide a non-empty fileNames
// parameter to a module which registers a file name provider.
//
// 2. If the file name provider returns a string which is empty, the
// MixFilter shall thenceforth return false.
//
// 3. If the file name provider returns a non-empty string that does
// not correspond to a readable file, an exception shall be thrown.
//
////////////////////////////////////////////////////////////////////////
// declareMixOp templates.
//
// These function templates should be used by writers of
// product-mixing "detail" classes to declare each product mix
// operation.
//
// All of the declareMixOp(...) function templates have the following
// template arguments:
//
// 1. The BranchType (defaults to art::InEvent). Specify explicitly if
//    you wish to mix subrun or run products, or if you need to
//    specify explicitly any of the other template arguments.
//
// 2. The incoming product type (deduced from the provided callable
//    mixer argument).
//
// 3. The outgoing product type (deduced from the provided callable
//    mixer argument).
//
// A product mixing operation should be specified by providing:
//
//  1. an InputTag specifying which secondary products should be mixed;
//
//  2. an optional instance label for the mixed product (defaulting to
//     the instance label of the incoming product if unspecified); and
//
//  3. a callable mixer such as:
//
//     bool mixfunc(std::vector<PROD const*> const&,
//                  OPROD&,
//                  PtrRemapper const&),
//
//     As the user may prefer, the mixer may take the form of:
//       a) an arbitrarily-named free function, or
//       b) a function object whose operator() must have a signature
//          whose arguments match those above, or
//       c) an arbitrarily-named member function of any class.
//     In each case, the mixer must have the same type (i.e.,
//     the same return type and the same parameter types) illustrated
//     by "mixfunc" above. The return value of the mix function is
//     taken to indicate whether the product should be placed in the
//     event.
//
//     Generally speaking the PROD and OPROD template arguments are
//     deducible from the callable mixer function which, once bound
//     (if appropriate) to any provided class instance, should be
//     convertible to an instantiation of the template instance
//     art::MixFunc<PROD, OPROD>. See
//     art/Framework/IO/ProductMix/MixTypes.h for more details.
//
//     If the provided function is a member function it may be
//     provided bound to the object upon which it is to be called by
//     the user (in which case it is treated as a free function by the
//     registration method) or by specifying the member function
//     followed by the object to which it should be bound (in which
//     case the bind will be done for the user). In this latter case
//     the template argument specifying the product type need *not* be
//     specified usually as it may be deduced from the signature of
//     the provided function. If one specifies an overload set however
//     (e.g. in the case where a class has several mix() member
//     functions, each one with a different mix function) then the
//     template argument must be specified in order to constrain the
//     overload set to a single function.
//
//     See also the description of the template alias
//     art::MixFunc<PROD, OPROD> defined in
//     art/Framework/IO/ProductMix/MixTypes.h.
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
//      function object (wrapped as a MixFunc<PROD, OPROD>).
//
//   2. Provide an InputTag, an output instance label, and a mixer
//      that is a free function or function object.
//
//   3. Provide an InputTag, a mixer that is a non-const member
//      function (of any class), and an object to which that member
//      function should be bound.
//
//   4. Provide an InputTag, an output instance label, a mixer that is
//      a non-const member function (of any class), and an object to
//      which that member function should be bound.
//
//   5. Same as 3, but providing a mixer that is a const member function.
//
//   6. Same as 4, but providing a mixer that is a const member function.
//
// Note: For signatures 3-6, if the compiler complains about an
// unresolved overload your first move should be to specify the
// product type(s) as template argument (to do this you must specify
// explicitly the BranchType template argument first). If that does
// not resolve the problem, try an explicit:
//
//   const_cast<T const&>(t)
//
// or
//
//   const_cast<T&>(t)
//
// as appropriate.
//
////////////////////////////////////
// produces() templates.
//
// Call as you would from the constructor of a module to declare
// (e.g., bookkeeping) products to be put into the event that are
// *not* direct results of a single product mix operation. For the
// latter case, see the declareMixOp() templates above.
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
#include "art/Framework/Core/Modifier.h"
#include "art/Framework/Core/PtrRemapper.h"
#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Framework/IO/ProductMix/MixOp.h"
#include "art/Framework/IO/ProductMix/MixTypes.h"
#include "art/Framework/IO/ProductMix/ProdToProdMapBuilder.h"
#include "art/Framework/Principal/fwd.h"
#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "art/Utilities/fwd.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "canvas/Persistency/Provenance/Compatibility/BranchIDList.h"
#include "canvas/Persistency/Provenance/FileFormatVersion.h"
#include "canvas/Persistency/Provenance/ProductID.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib/exempt_ptr.h"
#include "cetlib/value_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "Rtypes.h"
#include "TFile.h"
#include "TTree.h"

namespace art {
  class MixHelper;
}

class art::MixHelper : public art::detail::EngineCreator {
private:
  using ProviderFunc_ = std::function<std::string()>;

public:
  enum class Mode {
    SEQUENTIAL = 0,
    RANDOM_REPLACE,
    RANDOM_LIM_REPLACE,
    RANDOM_NO_REPLACE,
    UNKNOWN
  };

  struct Config {
    fhicl::Sequence<std::string> filenames{fhicl::Name{"fileNames"}, {}};
    fhicl::Atom<bool> compactMissingProducts{
      fhicl::Name{"compactMissingProducts"},
      false};
    fhicl::Atom<std::string> readMode{fhicl::Name{"readMode"}, "sequential"};
    fhicl::Atom<double> coverageFraction{fhicl::Name{"coverageFraction"}, 1.0};
    fhicl::Atom<bool> wrapFiles{fhicl::Name{"wrapFiles"}, false};
  };

  // Should probably pass in something like SharedModifier.
  MixHelper(Config const& config,
            std::string const& moduleLabel,
            Modifier& producesProvider);
  MixHelper(fhicl::ParameterSet const& pset,
            std::string const& moduleLabel,
            Modifier& producesProvider);

  // Returns the current mixing mode.
  Mode readMode() const;

  // Registers a callback to the detail object to determine the next
  // secondary file to read.
  void registerSecondaryFileNameProvider(ProviderFunc_ func);

  // A.
  template <class P>
  void produces(std::string const& instanceName = {});

  // B.
  template <class P, BranchType B>
  void produces(std::string const& instanceName = {});

  // 1.
  template <art::BranchType B = art::InEvent, typename PROD, typename OPROD>
  void declareMixOp(InputTag const& inputTag,
                    MixFunc<PROD, OPROD> mixFunc,
                    bool outputProduct = true);

  // 2.
  template <art::BranchType B = art::InEvent, typename PROD, typename OPROD>
  void declareMixOp(InputTag const& inputTag,
                    std::string const& outputInstanceLabel,
                    MixFunc<PROD, OPROD> mixFunc,
                    bool outputProduct = true);

  // 3.
  template <art::BranchType B = art::InEvent,
            typename PROD,
            typename OPROD,
            typename T>
  void declareMixOp(InputTag const& inputTag,
                    bool (T::*mixfunc)(std::vector<PROD const*> const&,
                                       OPROD&,
                                       PtrRemapper const&),
                    T& t,
                    bool outputProduct = true);

  // 4.
  template <art::BranchType B = art::InEvent,
            typename PROD,
            typename OPROD,
            typename T>
  void declareMixOp(InputTag const& inputTag,
                    std::string const& outputInstanceLabel,
                    bool (T::*mixfunc)(std::vector<PROD const*> const&,
                                       OPROD&,
                                       PtrRemapper const&),
                    T& t,
                    bool outputProduct = true);

  // 5.
  template <art::BranchType B = art::InEvent,
            typename PROD,
            typename OPROD,
            typename T>
  void declareMixOp(InputTag const& inputTag,
                    bool (T::*mixfunc)(std::vector<PROD const*> const&,
                                       OPROD&,
                                       PtrRemapper const&) const,
                    T const& t,
                    bool outputProduct = true);

  // 6.
  template <art::BranchType B = art::InEvent,
            typename PROD,
            typename OPROD,
            typename T>
  void declareMixOp(InputTag const& inputTag,
                    std::string const& outputInstanceLabel,
                    bool (T::*mixfunc)(std::vector<PROD const*> const&,
                                       OPROD&,
                                       PtrRemapper const&) const,
                    T const& t,
                    bool outputProduct = true);

  //////////////////////////////////////////////////////////////////////
  // Mix module writers should not need anything below this point.
  //////////////////////////////////////////////////////////////////////
  bool generateEventSequence(size_t nSecondaries,
                             EntryNumberSequence& enSeq,
                             EventIDSequence& eIDseq);
  void generateEventAuxiliarySequence(EntryNumberSequence const&,
                                      EventAuxiliarySequence&);
  void mixAndPut(EntryNumberSequence const& enSeq,
                 EventIDSequence const& eIDseq,
                 Event& e);
  void setEventsToSkipFunction(std::function<size_t()> eventsToSkip);

private:
  MixHelper(MixHelper const&) = delete;
  MixHelper& operator=(MixHelper const&) = delete;

  using MixOpList = std::vector<std::unique_ptr<MixOpBase>>;

  void initEngine_(fhicl::ParameterSet const& p);

  Mode initReadMode_(std::string const& mode) const;

  void openAndReadMetaData_(std::string fileName);
  bool openNextFile_();

  ProdToProdMapBuilder::ProductIDTransMap buildProductIDTransMap_(
    MixOpList& mixOps);

  Modifier& producesProvider_;
  std::vector<std::string> const filenames_;
  bool compactMissingProducts_;
  ProviderFunc_ providerFunc_{};
  MixOpList mixOps_{};
  PtrRemapper ptrRemapper_{};
  std::vector<std::string>::const_iterator fileIter_;
  Mode const readMode_;
  double const coverageFraction_;
  Long64_t nEventsReadThisFile_{};
  Long64_t nEventsInFile_{};
  Long64_t totalEventsRead_{};
  bool const canWrapFiles_;
  FileFormatVersion ffVersion_{};
  std::unique_ptr<art::BranchIDLists> branchIDLists_{
    nullptr}; // For backwards compatibility
  ProdToProdMapBuilder ptpBuilder_{};
  std::unique_ptr<CLHEP::RandFlat> dist_;
  std::function<size_t()> eventsToSkip_{};
  EntryNumberSequence shuffledSequence_{}; // RANDOM_NO_REPLACE only.
  bool haveSubRunMixOps_{false};
  bool haveRunMixOps_{false};

  // Root-specific state.
  cet::value_ptr<TFile> currentFile_{};
  cet::exempt_ptr<TTree> currentMetaDataTree_{nullptr};
  std::array<cet::exempt_ptr<TTree>, art::BranchType::NumBranchTypes>
    currentDataTrees_{{nullptr}};
  FileIndex currentFileIndex_{};
  std::array<RootBranchInfoList, art::BranchType::NumBranchTypes> dataBranches_{
    {}};
  EventIDIndex eventIDIndex_{};
};

inline auto
art::MixHelper::readMode() const -> Mode
{
  return readMode_;
}

// A.
template <class P>
inline void
art::MixHelper::produces(std::string const& instanceName)
{
  producesProvider_.produces<P>(instanceName);
}

// B.
template <class P, art::BranchType B>
inline void
art::MixHelper::produces(std::string const& instanceName)
{
  producesProvider_.produces<P, B>(instanceName);
}

// 1.
template <art::BranchType B, typename PROD, typename OPROD>
inline void
art::MixHelper::declareMixOp(InputTag const& inputTag,
                             MixFunc<PROD, OPROD> mixFunc,
                             bool outputProduct)
{
  declareMixOp<B>(inputTag, inputTag.instance(), mixFunc, outputProduct); // 2.
}

// 2.
template <art::BranchType B, typename PROD, typename OPROD>
void
art::MixHelper::declareMixOp(InputTag const& inputTag,
                             std::string const& outputInstanceLabel,
                             MixFunc<PROD, OPROD> mixFunc,
                             bool outputProduct)
{
  if (outputProduct) {
    produces<OPROD>(outputInstanceLabel);
  }
  if (B == art::InSubRun) {
    haveSubRunMixOps_ = true;
  } else if (B == art::InRun) {
    haveRunMixOps_ = true;
  }
  mixOps_.emplace_back(
    new MixOp<PROD, OPROD>(&producesProvider_.moduleDescription(),
                           inputTag,
                           outputInstanceLabel,
                           mixFunc,
                           outputProduct,
                           compactMissingProducts_,
                           B));
}

// 3.
template <art::BranchType B, typename PROD, typename OPROD, typename T>
inline void
art::MixHelper::declareMixOp(InputTag const& inputTag,
                             bool (T::*mixFunc)(std::vector<PROD const*> const&,
                                                OPROD&,
                                                PtrRemapper const&),
                             T& t,
                             bool outputProduct)
{
  declareMixOp<B>(inputTag,
                  inputTag.instance(),
                  mixFunc,
                  t,
                  outputProduct); // 4.
}

// 4.
template <art::BranchType B, typename PROD, typename OPROD, typename T>
void
art::MixHelper::declareMixOp(InputTag const& inputTag,
                             std::string const& outputInstanceLabel,
                             bool (T::*mixFunc)(std::vector<PROD const*> const&,
                                                OPROD&,
                                                PtrRemapper const&),
                             T& t,
                             bool outputProduct)
{
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  declareMixOp<B>(
    inputTag,
    outputInstanceLabel,
    static_cast<MixFunc<PROD, OPROD>>(std::bind(mixFunc, &t, _1, _2, _3)),
    outputProduct); // 2.
}

// 5.
template <art::BranchType B, typename PROD, typename OPROD, typename T>
inline void
art::MixHelper::declareMixOp(InputTag const& inputTag,
                             bool (T::*mixFunc)(std::vector<PROD const*> const&,
                                                OPROD&,
                                                PtrRemapper const&) const,
                             T const& t,
                             bool outputProduct)
{
  declareMixOp<B>(inputTag,
                  inputTag.instance(),
                  mixFunc,
                  t,
                  outputProduct); // 6.
}

// 6.
template <art::BranchType B, typename PROD, typename OPROD, typename T>
void
art::MixHelper::declareMixOp(InputTag const& inputTag,
                             std::string const& outputInstanceLabel,
                             bool (T::*mixFunc)(std::vector<PROD const*> const&,
                                                OPROD&,
                                                PtrRemapper const&) const,
                             T const& t,
                             bool outputProduct)
{
  using std::placeholders::_1;
  using std::placeholders::_2;
  using std::placeholders::_3;
  declareMixOp<B>(
    inputTag,
    outputInstanceLabel,
    static_cast<MixFunc<PROD, OPROD>>(std::bind(mixFunc, &t, _1, _2, _3)),
    outputProduct); // 2.
}

#endif /* art_Framework_IO_ProductMix_MixHelper_h */

// Local Variables:
// mode: c++
// End:
