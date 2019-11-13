#ifndef art_Framework_Modules_MixFilter_h
#define art_Framework_Modules_MixFilter_h

////////////////////////////////////////////////////////////////////////
//
// The MixFilter class template is used to create filters capable of
// mixing products from a secondary event (or subrun, or run) stream
// into the primary event.
//
// The MixFilter class template requires two template arguments:
//
//    - the use of a type T as its
// template parameter; this type T must supply the following non-static
// member functions:
//
//    T(fhicl::ParameterSet const& p, art::MixHelper& helper);
//
//    // Construct an object of type T. The ParameterSet provided will
//    // be the configuration of the module constructing the T. It is
//    // recommended (but not enforced) that detail parameters be placed
//    // in their own (eg "detail") ParameterSet to reduce the potential
//    // for clashes with parameters used by the template. The helper
//    // is not copyable and must be used in the constructor to
//    // register:
//    //
//    // a. Any mixing operations to be carried out by this module by
//    //    means of declareMixOp<> calls.
//    //
//    // b. Any non-mix (e.g. bookkeeping) products by means of
//    //    produces<> calls.
//    //
//    // Further details may be found in
//    // art/Framework/IO/ProductMix/MixHelper.h.
//
//    size_t nSecondaries();
//
//    // Provide the number of secondary events to be mixed into the
//    // current primary event.
//
// In addition, T may optionally provide any or all of the following
// member functions; each will be called at the appropriate time iff
// it is declared.  (There must, of course, be a function definition
// corresponding to each declared function.)
//
//    void startEvent(art::Event const& e);
//
//    // Reset internal cache information at the start of the current
//    // event.
//
//    size_t eventsToSkip();
//
//    // Provide the number of secondary events at the beginning of the
//    // next secondary input file that should be skipped. Note:
//    // may be declare const or not as appropriate.
//
//    void processEventIDs(art::EventIDSequence const& seq);
//
//    // Receive the ordered sequence of EventIDs that will be mixed into
//    // the current event; useful for bookkeeping purposes.
//
//    void processEventAuxiliaries(art::EventAuxiliarySequence const& seq);
//
//    // Receive the ordered sequence of EventAuxiliaries that will be mixed
//    // into the current event; useful for bookkeeping purposes.
//
//    void finalizeEvent(art::Event& e);
//
//    // Do end-of-event tasks (e.g., inserting bookkeeping data products into
//    // the primary event).
//
//    void respondToOpenInputFile(FileBlock const& fb);
//
//    // Called when a new primary input file is opened.
//
//    void respondToCloseInputFile(FileBlock const& fb);
//
//    // Called when a primary input file is closed.
//
//    void respondToOpenOutputFiles(FileBlock const& fb);
//
//    // Called when a new output file is opened.
//
//    void respondToCloseOutputFiles(FileBlock const& fb);
//
//    // Called when an output file is closed.
//
//    void beginSubRun(art::SubRun const& sr);
//
//    // Do beginning-of-subrun tasks.
//
//    void endSubRun(art::SubRun& sr);
//
//    // Do end-of-subrun tasks (e.g. insert products into the primary subrun).
//
//    void beginRun(art::Run const& r);
//
//    // Do beginning-of-run tasks.
//
//    void endRun(art::Run& r);
//
//    // Do end-of-run tasks (e.g. insert products into the primary run).
//
////////////////////////////////////////////////////////////////////////
// Notes.
//
// 1. Functions declared to the MixHelper to actually carry out the
//    mixing of the products may be (1) member functions of this or
//    another class; or (2) free functions (including bound functions)
//    or (3) function objects.
//
// 2. It is possible to declare mix operations which produce an output
//    product of a different type to that of the mixed input products
//    via the provision of an appropriate mixer function: see the
//    documentation in art/Framework/IO/ProductMix/MixHelper.h for more
//    details.
//
// 3. It is possible to declare mix operations which take as input
//    products from the subrun or run streams. Some points to note:
//
//    * The mix operation so declared will be invoked upon every primary
//      event.
//
//    * There will be one provided product for every mixed event from
//      the secondary stream, regardless of duplication. It is the
//      responsibility of the user's mixing function to deal with the
//      provided information appropriately. We recommend making use of
//      the optional processEventIDs(...) and
//      processEventAuxiliaries(...) functions to avoid unintentional
//      double-counting of information.
//
//    * The mix operation may generate event-level data for the primary
//      stream from the provided subrun and run products exactly as if
//      it were a normal event-level mix operation. Any production of
//      subrun- or run-level data from the primary stream should be done
//      in the optional endSubRun(..) and / or endRun(...) functions as
//      appropriate. Any such subrun- or run-level products should be
//      declared appropriately via art::MixHelper::products(...).
//
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/IO/ProductMix/MixTypes.h"
#include "canvas/Persistency/Provenance/BranchType.h"
#include "cetlib/metaprogramming.h"
#include "fhiclcpp/types/TableFragment.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <functional>
#include <memory>
#include <type_traits>

namespace art {
  template <typename T, typename IOPolicy>
  class MixFilter;

  template <typename T>
  class MixFilterTable {
  public:
    using value_type = T;

    auto const&
    operator()() const
    {
      return fragment_();
    }

  private:
    fhicl::TableFragment<T> fragment_;
  };

  namespace detail {
    // Template metaprogramming.
    using cet::enable_if_function_exists_t;

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void startEvent()?

    template <typename T, typename = void>
    struct has_startEvent : std::false_type {};

    template <typename T>
    struct has_startEvent<
      T,
      enable_if_function_exists_t<void (T::*)(Event const&), &T::startEvent>>
      : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method size_t eventsToSkip() const?

    template <typename T, typename = void>
    struct has_eventsToSkip : std::false_type {};

    template <typename T>
    struct has_eventsToSkip<
      T,
      enable_if_function_exists_t<size_t (T::*)(), &T::eventsToSkip>>
      : std::true_type {};

    template <typename T>
    struct has_eventsToSkip<
      T,
      enable_if_function_exists_t<size_t (T::*)() const, &T::eventsToSkip>>
      : std::true_type {};
    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void
    // processEventIDs(EventIDSequence const&)?

    template <typename T, typename = void>
    struct has_processEventIDs : std::false_type {};

    template <typename T>
    struct has_processEventIDs<
      T,
      enable_if_function_exists_t<void (T::*)(EventIDSequence const&),
                                  &T::processEventIDs>> : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void
    // processEventAuxiliaries(EventAuxiliarySequence const&)?

    template <typename T, typename = void>
    struct has_processEventAuxiliaries : std::false_type {};

    template <typename T>
    struct has_processEventAuxiliaries<
      T,
      enable_if_function_exists_t<void (T::*)(EventAuxiliarySequence const&),
                                  &T::processEventAuxiliaries>>
      : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void finalizeEvent(Event&)?
    template <typename T, typename = void>
    struct has_finalizeEvent : std::false_type {};

    template <typename T>
    struct has_finalizeEvent<
      T,
      enable_if_function_exists_t<void (T::*)(Event&), &T::finalizeEvent>>
      : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void beginSubRun(SubRun const&)?
    template <typename T, typename = void>
    struct has_beginSubRun : std::false_type {};

    template <typename T>
    struct has_beginSubRun<
      T,
      enable_if_function_exists_t<void (T::*)(SubRun const&), &T::beginSubRun>>
      : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void endSubRun(SubRun&)?
    template <typename T, typename = void>
    struct has_endSubRun : std::false_type {};

    template <typename T>
    struct has_endSubRun<
      T,
      enable_if_function_exists_t<void (T::*)(SubRun&), &T::endSubRun>>
      : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void beginRun(Run const&)?
    template <typename T, typename = void>
    struct has_beginRun : std::false_type {};

    template <typename T>
    struct has_beginRun<
      T,
      enable_if_function_exists_t<void (T::*)(Run const&), &T::beginRun>>
      : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void endRun(Run&)?
    template <typename T, typename = void>
    struct has_endRun : std::false_type {};

    template <typename T>
    struct has_endRun<
      T,
      enable_if_function_exists_t<void (T::*)(Run&), &T::endRun>>
      : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have respondToXXX methods()?
    template <typename T>
    using respond_to_file = void (T::*)(FileBlock const&);

    template <typename T, respond_to_file<T>>
    struct respondToXXX_function;

    // has_respondToOpenInputFile
    template <typename T, typename = void>
    struct has_respondToOpenInputFile : std::false_type {};

    template <typename T>
    struct has_respondToOpenInputFile<
      T,
      enable_if_function_exists_t<respond_to_file<T>,
                                  &T::respondToOpenInputFile>>
      : std::true_type {};

    // has_respondToCloseInputFile
    template <typename T, typename = void>
    struct has_respondToCloseInputFile : std::false_type {};

    template <typename T>
    struct has_respondToCloseInputFile<
      T,
      enable_if_function_exists_t<respond_to_file<T>,
                                  &T::respondToCloseInputFile>>
      : std::true_type {};

    // has_respondToOpenOutputFiles
    template <typename T, typename = void>
    struct has_respondToOpenOutputFiles : std::false_type {};

    template <typename T>
    struct has_respondToOpenOutputFiles<
      T,
      enable_if_function_exists_t<respond_to_file<T>,
                                  &T::respondToOpenOutputFiles>>
      : std::true_type {};

    // has_respondToCloseOutputFiles
    template <typename T, typename = void>
    struct has_respondToCloseOutputFiles : std::false_type {};

    template <typename T>
    struct has_respondToCloseOutputFiles<
      T,
      enable_if_function_exists_t<respond_to_file<T>,
                                  &T::respondToCloseOutputFiles>>
      : std::true_type {};

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a Parameters type?
    template <typename T, typename = void>
    struct maybe_has_Parameters : std::false_type {
      using Parameters = fhicl::ParameterSet;
    };

    template <typename T>
    struct maybe_has_Parameters<T, std::void_t<typename T::Parameters>>
      : std::true_type {
      using user_config_t = typename T::Parameters;
      struct Config {
        fhicl::TableFragment<MixHelper::Config> mixHelper;
        user_config_t userConfig;
      };
      using Parameters = EDFilter::Table<Config>;
    };

  } // namespace detail

} // namespace art

template <typename T, typename IOPolicy>
class art::MixFilter : public EDFilter {
public:
  using MixDetail = T;

  using Parameters = typename detail::maybe_has_Parameters<T>::Parameters;

  template <typename U = Parameters>
  explicit MixFilter(std::enable_if_t<std::is_same_v<U, fhicl::ParameterSet>,
                                      fhicl::ParameterSet> const& p);
  template <typename U = Parameters>
  explicit MixFilter(
    std::enable_if_t<!std::is_same_v<U, fhicl::ParameterSet>, U> const& p);

private:
  void respondToOpenInputFile(FileBlock const& fb) override;
  void respondToCloseInputFile(FileBlock const& fb) override;
  void respondToOpenOutputFiles(FileBlock const& fb) override;
  void respondToCloseOutputFiles(FileBlock const& fb) override;
  bool filter(Event& e) override;
  bool beginSubRun(SubRun& sr) override;
  bool endSubRun(SubRun& sr) override;
  bool beginRun(Run& r) override;
  bool endRun(Run& r) override;

  MixHelper helper_;
  MixDetail detail_;
};

template <typename T, typename IOPolicy>
template <typename U>
art::MixFilter<T, IOPolicy>::MixFilter(
  std::enable_if_t<std::is_same_v<U, fhicl::ParameterSet>,
                   fhicl::ParameterSet> const& p)
  : EDFilter{p}
  , helper_{p,
            p.template get<std::string>("module_label"),
            producesCollector(),
            std::make_unique<IOPolicy>()}
  , detail_{p, helper_}
{
  if constexpr (detail::has_eventsToSkip<T>::value) {
    helper_.setEventsToSkipFunction([this] { return detail_.eventsToSkip(); });
  }
}

template <typename T, typename IOPolicy>
template <typename U>
art::MixFilter<T, IOPolicy>::MixFilter(
  std::enable_if_t<!std::is_same_v<U, fhicl::ParameterSet>, U> const& p)
  : EDFilter{p}
  , helper_{p().mixHelper(),
            p.get_PSet().template get<std::string>("module_label"),
            producesCollector(),
            std::make_unique<IOPolicy>()}
  , detail_{p().userConfig, helper_}
{
  if constexpr (detail::has_eventsToSkip<T>::value) {
    helper_.setEventsToSkipFunction([this] { return detail_.eventsToSkip(); });
  }
}

template <typename T, typename IOPolicy>
void
art::MixFilter<T, IOPolicy>::respondToOpenInputFile(FileBlock const& fb)
{
  if constexpr (detail::has_respondToOpenInputFile<T>::value) {
    detail_.respondToOpenInputFile(fb);
  }
}

template <typename T, typename IOPolicy>
void
art::MixFilter<T, IOPolicy>::respondToCloseInputFile(FileBlock const& fb)
{
  if constexpr (detail::has_respondToCloseInputFile<T>::value) {
    detail_.respondToCloseInputFile(fb);
  }
}

template <typename T, typename IOPolicy>
void
art::MixFilter<T, IOPolicy>::respondToOpenOutputFiles(FileBlock const& fb)
{
  if constexpr (detail::has_respondToOpenOutputFiles<T>::value) {
    detail_.respondToOpenOutputFiles(fb);
  }
}

template <typename T, typename IOPolicy>
void
art::MixFilter<T, IOPolicy>::respondToCloseOutputFiles(FileBlock const& fb)
{
  if constexpr (detail::has_respondToCloseOutputFiles<T>::value) {
    detail_.respondToCloseOutputFiles(fb);
  }
}

template <typename T, typename IOPolicy>
bool
art::MixFilter<T, IOPolicy>::filter(Event& e)
{
  // 1. Call detail object's startEvent() if it exists.
  if constexpr (detail::has_startEvent<T>::value) {
    detail_.startEvent(e);
  }

  // 2. Ask detail object how many events to read.
  size_t const nSecondaries = detail_.nSecondaries();

  // 3. Decide which events we're reading and prime the event tree
  //    cache.
  EntryNumberSequence enSeq;
  EventIDSequence eIDseq;
  enSeq.reserve(nSecondaries);
  eIDseq.reserve(nSecondaries);
  if (!helper_.generateEventSequence(nSecondaries, enSeq, eIDseq)) {
    throw Exception(errors::FileReadError)
      << "Insufficient secondary events available to mix.\n";
  }

  // 4. Give the event ID sequence to the detail object.
  if constexpr (detail::has_processEventIDs<T>::value) {
    detail_.processEventIDs(eIDseq);
  }

  // 5. Give the event auxiliary sequence to the detail object.
  if constexpr (detail::has_processEventAuxiliaries<T>::value) {
    auto const auxseq = helper_.generateEventAuxiliarySequence(enSeq);
    detail_.processEventAuxiliaries(auxseq);
  }

  // 6. Make the MixHelper read info into all the products, invoke the
  //    mix functions and put the products into the event.
  helper_.mixAndPut(enSeq, eIDseq, e);

  // 7. Call detail object's finalizeEvent() if it exists.
  if constexpr (detail::has_finalizeEvent<T>::value) {
    detail_.finalizeEvent(e);
  }
  return true;
}

template <typename T, typename IOPolicy>
bool
art::MixFilter<T, IOPolicy>::beginSubRun(SubRun& sr)
{
  if constexpr (detail::has_beginSubRun<T>::value) {
    detail_.beginSubRun(sr);
  }
  return true;
}

template <typename T, typename IOPolicy>
bool
art::MixFilter<T, IOPolicy>::endSubRun(SubRun& sr)
{
  if constexpr (detail::has_endSubRun<T>::value) {
    detail_.endSubRun(sr);
  }
  return true;
}

template <typename T, typename IOPolicy>
bool
art::MixFilter<T, IOPolicy>::beginRun(Run& r)
{
  if constexpr (detail::has_beginRun<T>::value) {
    detail_.beginRun(r);
  }
  return true;
}

template <typename T, typename IOPolicy>
bool
art::MixFilter<T, IOPolicy>::endRun(Run& r)
{
  if constexpr (detail::has_endRun<T>::value) {
    detail_.endRun(r);
  }
  return true;
}

#endif /* art_Framework_Modules_MixFilter_h */

// Local Variables:
// mode: c++
// End:
