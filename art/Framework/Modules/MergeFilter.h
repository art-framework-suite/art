#ifndef art_Framework_Modules_MergeFilter_h
#define art_Framework_Modules_MergeFilter_h

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/IO/ProductMerge/MergeHelper.h"
#include "art/Framework/IO/ProductMerge/MergeHelper.h"
#include "cpp0x/type_traits"

namespace art {
  template <class T>
  class MergeFilter;

  namespace detail {
    // Template metaprogramming.
    typedef char (& no_tag )[1]; // type indicating FALSE
    typedef char (& yes_tag)[2]; // type indicating TRUE

    ////////////////////////////////////////////////////////////////////
    // Does the detail object have a method void startEvent()?
    template <typename T, void (T::*)()> struct startEvent_function;

    template <typename T> struct void_do_not_call_startEvent {
    public:
      void operator()(T &t) {}
    };

    template <typename T> struct call_startEvent {
    public:
      void operator()(T &t) { t.startEvent(); }
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
    // Does the detail object have a method void finalizeEvent(Event&)?
    template <typename T, void (T::*)(Event &)> struct finalizeEvent_function;

    template <typename T> struct void_do_not_call_finalizeEvent {
    public:
      void operator()(T &, Event &) {}
    };

    template <typename T> struct call_finalizeEvent {
    public:
      void operator()(T &t, Event &e) { t.finalizeEvent(e); }
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

  } // detail namespace

} // art namespace

template <class T>
class art::MergeFilter : public art::EDFilter {
public:
  typedef T MergeDetail;
  explicit MergeFilter(fhicl::ParameterSet const &p);

  virtual void beginJob();
  virtual bool filter(art::Event &e);

private:

  MergeHelper helper_;
  MergeDetail detail_;
};

template <class T>
art::MergeFilter<T>::MergeFilter(fhicl::ParameterSet const &p)
  :
  EDFilter(),
  helper_(p, *this),
  detail_(p, helper_)
{
}

template <class T>
void
art::MergeFilter<T>::beginJob() {
  helper_.postRegistrationInit();
}

template <class T>
bool
art::MergeFilter<T>::filter(art::Event &e) {
  // 1. Call detail.startEvent() if it exists.
  typename std::conditional<detail::has_startEvent<T>::value, detail::call_startEvent<T>, detail::void_do_not_call_startEvent<T> >::type maybe_call_startEvent;
  maybe_call_startEvent(detail_);

  // 2. Ask detail object how many events to read.
  size_t nSecondaries = detail_.nSecondaries();

  // 3. Make the MergeHelper read info into all the products, invoke the
  // merge functions and put the products into the event.
  helper_.mergeAndPut(nSecondaries, e);

  // 4. Call detail.finalizeEvent() if it exists.
  typename std::conditional<detail::has_finalizeEvent<T>::value, detail::call_finalizeEvent<T>, detail::void_do_not_call_finalizeEvent<T> >::type maybe_call_finalizeEvent;
  maybe_call_finalizeEvent(detail_, e);
  return false;
}

// }
#endif /* art_Framework_Modules_MergeFilter_h */

// Local Variables:
// mode: c++
// End:
