#ifndef art_Framework_Modules_MixFilter_h
#define art_Framework_Modules_MixFilter_h

// ======================================================================
//
// The MixFilter class template is used to create filters capable of
// mixing products from a secondary event stream into the primary event.
//
// The MixFilter class template requires the use of a type T as its
// template parameter; this type T must supply the following non-static
// member functions:
//
//    T(fhicl::ParameterSet const &p, art::MixHelper &helper);
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
//    // art/Framework/ProductMix/MixHelper.h.
//
//    size_t nSecondaries() const;
//
//    // Provide the number of secondary events to be mixed into the
//    // current primary event.
//
// In addition, T may optionally provide any or all of the following
// member functions; each will be called at the appropriate time iff
// it is declared.  (There must, of course, be a function definition
// corresponding to each declared function.)
//
//    void startEvent(art::Event const &e);
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
//    void processEventIDs(art::EventIDSequence const &seq);
//
//    // Receive the ordered sequence of EventIDs that will be mixed into
//    // the current event; useful for bookkeeping purposes.
//
//    void finalizeEvent(art::Event &t);
//
//    // Do end-of-event tasks (e.g., inserting bookkeeping objects into
//    // the primary event).
//
// Functions declared to the MixHelper to actually carry out the mixing
// of the products may be (1) member functions of this or another class;
// or (2) free functions or (3) function objects.
// ======================================================================

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/IO/ProductMix/MixContainerTypes.h"
#include "art/Framework/IO/ProductMix/MixHelper.h"
#include "art/Framework/IO/ProductMix/MixOpBase.h"
#include "art/Framework/Services/Registry/ServiceRegistry.h"
#include "canvas/Utilities/detail/metaprogramming.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <functional>
#include <type_traits>

namespace art {
  template <class T>
  class MixFilter;

  namespace detail {
    // Template metaprogramming.

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void startEvent()?
    template <typename T, void (T:: *)(Event const &)> struct startEvent_function;

    //////////
    // Temporary trapping of old interface in user code.
    template <typename X, void (X:: *)()> struct old_startEvent_function;
    template <typename X>
    no_tag
    has_old_startEvent_helper(...);
    template <typename X>
    yes_tag
    has_old_startEvent_helper(old_startEvent_function<X, &X::startEvent> * dummy);
    template <typename X>
    struct has_old_startEvent {
      static bool const value =
        sizeof(has_old_startEvent_helper<X>(0)) == sizeof(yes_tag);
    };

    template <typename T> struct do_call_old_startEvent {
    public:
      void operator()(T & t) {
        static bool need_warning = true;
        if (need_warning) {
          mf::LogWarning("Deprecated")
            << "Mixing driver function has signature startEvent(), which is deprecated.\n"
            << "Please update your code to define startEvent(Event const &).\n"
            << "In a future version of ART the old method will no longer be called.";
          need_warning = false;
        }
        t.startEvent();
      }
    };

    template <typename T> struct do_not_call_old_startEvent {
      void operator()(T&) { }
    };

    template <typename T> struct do_not_call_startEvent {
    public:
      do_not_call_startEvent(Event const &) { }
      void operator()(T & t) {
        std::conditional_t<has_old_startEvent<T>::value,
                           do_call_old_startEvent<T>,
                           do_not_call_old_startEvent<T> > maybe_call_old_startEvent;
        maybe_call_old_startEvent(t);
      }
    private:
    };
    //////////

    template <typename T> struct call_startEvent {
    public:
      call_startEvent(Event const & e) : e_(e) { }
      void operator()(T & t) { t.startEvent(e_); }
    private:
      Event const & e_;
    };

    template <typename T>
    no_tag
    has_startEvent_helper(...);

    template <typename T>
    yes_tag
    has_startEvent_helper(startEvent_function<T, &T::startEvent> * dummy);

    template <typename T>
    struct has_startEvent {
      static bool const value =
        sizeof(has_startEvent_helper<T>(0)) == sizeof(yes_tag);
    };
    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method size_t eventsToSkip() const?
    template <typename T, size_t (T:: *)()> struct eventsToSkip_function;
    template <typename T, size_t (T:: *)() const> struct const_eventsToSkip_function;

    template <typename T> struct do_not_setup_eventsToSkip {
      do_not_setup_eventsToSkip(MixHelper &, T &) { }
    };

    template <typename T>
    size_t
    call_eventsToSkip(T & t) { return t.eventsToSkip(); }

    template <typename T> struct setup_eventsToSkip {
      setup_eventsToSkip(MixHelper & helper, T & t) {
        helper.setEventsToSkipFunction(std::bind(&detail::call_eventsToSkip<T>, std::ref(t)));
      }
    };

    template <typename T>
    no_tag
    has_eventsToSkip_helper(...);

    template <typename T>
    yes_tag
    has_eventsToSkip_helper(eventsToSkip_function<T, &T::eventsToSkip> *);

    template <typename T>
    yes_tag
    has_eventsToSkip_helper(const_eventsToSkip_function<T, &T::eventsToSkip> *);

    template <typename T> struct has_eventsToSkip {
      static bool const value =
        sizeof(has_eventsToSkip_helper<T>(0)) == sizeof(yes_tag);
    };

    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void
    // processEventIDs(EventIDSequence const &)?
    template <typename T, void (T:: *)(EventIDSequence const &)> struct processEventIDs_function;

    template <typename T> struct do_not_call_processEventIDs {
    public:
      void operator()(T &, EventIDSequence const &) {}
    };

    template <typename T> struct call_processEventIDs {
    public:
      void operator()(T & t, EventIDSequence const & seq) { t.processEventIDs(seq); }
    };

    template <typename T>
    no_tag
    has_processEventIDs_helper(...);

    template <typename T>
    yes_tag
    has_processEventIDs_helper(processEventIDs_function<T, &T::processEventIDs> * dummy);

    template <typename T>
    struct has_processEventIDs {
      static bool const value =
        sizeof(has_processEventIDs_helper<T>(0)) == sizeof(yes_tag);
    };
    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void finalizeEvent(Event&)?
    template <typename T, void (T:: *)(Event &)> struct finalizeEvent_function;

    template <typename T> struct do_not_call_finalizeEvent {
    public:
      void operator()(T &, Event &) {}
    };

    template <typename T> struct call_finalizeEvent {
    public:
      void operator()(T & t, Event & e) { t.finalizeEvent(e); }
    };

    template <typename T>
    no_tag
    has_finalizeEvent_helper(...);

    template <typename T>
    yes_tag
    has_finalizeEvent_helper(finalizeEvent_function<T, &T::finalizeEvent> * dummy);

    template <typename T>
    struct has_finalizeEvent {
      static bool const value =
        sizeof(has_finalizeEvent_helper<T>(0)) == sizeof(yes_tag);
    };
    ////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have respondToXXX methods()?
    template <typename T, void (T:: *)(FileBlock const &)>
    struct respondToXXX_function;

    template <typename T> struct do_not_call_respondToXXX {
      do_not_call_respondToXXX(T &, FileBlock const &) {}
    };

    template <typename T> struct call_respondToOpenInputFile {
      call_respondToOpenInputFile(T & t, FileBlock const & fb) {
        t.respondToOpenInputFile(fb);
      }
    };

    template <typename T> struct call_respondToCloseInputFile {
      call_respondToCloseInputFile(T & t, FileBlock const & fb) {
        t.respondToCloseInputFile(fb);
      }
    };

    template <typename T> struct call_respondToOpenOutputFiles {
      call_respondToOpenOutputFiles(T & t, FileBlock const & fb) {
        t.respondToOpenOutputFiles(fb);
      }
    };

    template <typename T> struct call_respondToCloseOutputFiles {
      call_respondToCloseOutputFiles(T & t, FileBlock const & fb) {
        t.respondToCloseOutputFiles(fb);
      }
    };

    template <typename T>
    no_tag
    has_respondToOpenInputFile_helper(...);

    template <typename T>
    yes_tag
    has_respondToOpenInputFile_helper(respondToXXX_function<T, &T::respondToOpenInputFile> * dummy);

    template <typename T>
    no_tag
    has_respondToCloseInputFile_helper(...);

    template <typename T>
    yes_tag
    has_respondToCloseInputFile_helper(respondToXXX_function<T, &T::respondToCloseInputFile> * dummy);

    template <typename T>
    no_tag
    has_respondToOpenOutputFiles_helper(...);

    template <typename T>
    yes_tag
    has_respondToOpenOutputFiles_helper(respondToXXX_function<T, &T::respondToOpenOutputFiles> * dummy);

    template <typename T>
    no_tag
    has_respondToCloseOutputFiles_helper(...);

    template <typename T>
    yes_tag
    has_respondToCloseOutputFiles_helper(respondToXXX_function<T, &T::respondToCloseOutputFiles> * dummy);

    template <typename T>
    struct has_respondToOpenInputFile {
      static bool const value =
        sizeof(has_respondToOpenInputFile_helper<T>(0)) == sizeof(yes_tag);
    };

    template <typename T>
    struct has_respondToCloseInputFile {
      static bool const value =
        sizeof(has_respondToCloseInputFile_helper<T>(0)) == sizeof(yes_tag);
    };

    template <typename T>
    struct has_respondToOpenOutputFiles {
      static bool const value =
        sizeof(has_respondToOpenOutputFiles_helper<T>(0)) == sizeof(yes_tag);
    };

    template <typename T>
    struct has_respondToCloseOutputFiles {
      static bool const value =
        sizeof(has_respondToCloseOutputFiles_helper<T>(0)) == sizeof(yes_tag);
    };
    ////////////////////////////////////////////////////////////////////

  } // detail namespace

} // art namespace

template <class T>
class art::MixFilter : public art::EDFilter {
public:
  typedef T MixDetail;
  explicit MixFilter(fhicl::ParameterSet const & p);

  void beginJob() override;
  void respondToOpenInputFile(FileBlock const & fb) override;
  void respondToCloseInputFile(FileBlock const & fb) override;
  void respondToOpenOutputFiles(FileBlock const & fb) override;
  void respondToCloseOutputFiles(FileBlock const & fb) override;
  bool filter(art::Event & e) override;

private:
  fhicl::ParameterSet const &
  initEngine(fhicl::ParameterSet const & p);
  MixHelper helper_;
  MixDetail detail_;
};

template <class T>
art::MixFilter<T>::MixFilter(fhicl::ParameterSet const & p)
  :
  EDFilter(),
  helper_(initEngine(p), *this), // See note below
  detail_(p, helper_)
{
  // Note that the random number engine is created in the initializer
  // list by calling initEngine(). This enables the engine to be
  // obtained by the helper and or detail objects in their constructors
  // via a service handle to the random number generator service. The
  // initEngine() function returns a ParameterSet simply so that it may
  // be called in this place without having to resort to comma-separated
  // bundles to do the job.
  std::conditional_t<detail::has_eventsToSkip<T>::value, detail::setup_eventsToSkip<T>, detail::do_not_setup_eventsToSkip<T> >
    maybe_setup_skipper(helper_, detail_);
}

template <class T>
void
art::MixFilter<T>::beginJob()
{
}

template <class T>
void
art::MixFilter<T>::respondToOpenInputFile(FileBlock const & fb)
{
  std::conditional_t<detail::has_respondToOpenInputFile<T>::value,
                     detail::call_respondToOpenInputFile<T>,
                     detail::do_not_call_respondToXXX<T>>
    (detail_, fb);
}

template <class T>
void
art::MixFilter<T>::respondToCloseInputFile(FileBlock const & fb)
{
  std::conditional_t<detail::has_respondToCloseInputFile<T>::value,
                     detail::call_respondToCloseInputFile<T>,
                     detail::do_not_call_respondToXXX<T>>
    (detail_, fb);
}

template <class T>
void
art::MixFilter<T>::respondToOpenOutputFiles(FileBlock const & fb)
{
  std::conditional_t<detail::has_respondToOpenOutputFiles<T>::value,
                     detail::call_respondToOpenOutputFiles<T>,
                     detail::do_not_call_respondToXXX<T>>
    (detail_, fb);
}

template <class T>
void
art::MixFilter<T>::respondToCloseOutputFiles(FileBlock const & fb)
{
  std::conditional_t <detail::has_respondToCloseOutputFiles<T>::value,
                      detail::call_respondToCloseOutputFiles<T>,
                      detail::do_not_call_respondToXXX<T>>
    (detail_, fb);
}

template <class T>
bool
art::MixFilter<T>::filter(art::Event & e)
{
  // 1. Call detail object's startEvent() if it exists.
  std::conditional_t < detail::has_startEvent<T>::value,
                       detail::call_startEvent<T>,
                       detail::do_not_call_startEvent<T> > maybe_call_startEvent(e);
  maybe_call_startEvent(detail_);
  // 2. Ask detail object how many events to read.
  size_t nSecondaries = detail_.nSecondaries();
  // 3. Decide which events we're reading and prime the event tree cache.
  EntryNumberSequence enSeq;
  EventIDSequence eIDseq;
  enSeq.reserve(nSecondaries);
  eIDseq.reserve(nSecondaries);
  if (!helper_.generateEventSequence(nSecondaries, enSeq, eIDseq)) {
    throw Exception(errors::FileReadError)
      << "Insufficient secondary events available to mix.\n";
  }
  // 4. Give the event ID sequence to the detail object.
  std::conditional_t < detail::has_processEventIDs<T>::value,
                       detail::call_processEventIDs<T>,
                       detail::do_not_call_processEventIDs<T> > maybe_call_processEventIDs;
  maybe_call_processEventIDs(detail_, eIDseq);
  // 5. Make the MixHelper read info into all the products, invoke the
  // mix functions and put the products into the event.
  helper_.mixAndPut(enSeq, e);
  // 6. Call detail object's finalizeEvent() if it exists.
  std::conditional_t < detail::has_finalizeEvent<T>::value,
                       detail::call_finalizeEvent<T>,
                       detail::do_not_call_finalizeEvent<T> >
    maybe_call_finalizeEvent;
  maybe_call_finalizeEvent(detail_, e);
  return true;
}

template <class T>
fhicl::ParameterSet const &
art::MixFilter<T>::initEngine(fhicl::ParameterSet const & p) {
  // If we can't create one of these, the helper will deal with the
  // situation accordingly.
  if (ServiceRegistry::instance().isAvailable<RandomNumberGenerator>()) {
    createEngine(get_seed_value(p));
  }
  return p;
}

#endif /* art_Framework_Modules_MixFilter_h */

// Local Variables:
// mode: c++
// End:
