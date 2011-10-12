#ifndef art_Framework_Core_detail_getAssnsHandle_h
#define art_Framework_Core_detail_getAssnsHandle_h

// Utility functions to obtain the desired Assns object from the event.

#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/Assns.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/InputTag.h"

namespace art {
  namespace detail {
    template <typename ProdA, typename ProdB>
    Handle<Assns<ProdA, ProdB, void> >
    getAssnsHandle(Event const & e, InputTag const & tag);

    template <typename ProdA, typename ProdB, typename Data>
    Handle<Assns<ProdA, ProdB, Data> >
    getAssnsHandle(Event const & e, InputTag const & tag);

    template <typename ProdA, typename ProdB, typename Data = void>
    struct GetAssnsHandle {
      Handle<Assns<ProdA, ProdB, Data> >
      operator()(Event const & e, InputTag const & tag) const;
    };
  }
}

template <typename ProdA, typename ProdB>
inline
art::Handle <art::Assns<ProdA, ProdB, void> >
art::detail::getAssnsHandle(Event const & e, InputTag const & tag)
{
  return GetAssnsHandle<ProdA, ProdB, void>()(e, tag);
}

template <typename ProdA, typename ProdB, typename Data>
inline
art::Handle <art::Assns<ProdA, ProdB, Data> >
art::detail::getAssnsHandle(Event const & e, InputTag const & tag)
{
  return GetAssnsHandle<ProdA, ProdB, Data>()(e, tag);
}

template <typename ProdA, typename ProdB, typename Data>
art::Handle <art::Assns<ProdA, ProdB, Data> >
art::detail::GetAssnsHandle<ProdA, ProdB, Data>::
operator()(Event const & e, InputTag const & tag) const
{
  Handle<Assns<ProdA, ProdB, Data> > h;
  e.getByLabel(tag, h);
  if (h.failedToGet()) {
    try { // Only way to get the exception is to cause it to be thrown:
      h.product();
    }
    catch (Exception & e) {
      if (e.categoryCode() == errors::ProductNotFound) {
        throw Exception(errors::ProductNotFound, "FindOne")
            << "Unable to find an appropriate association collection as specified"
            << " by tag: "
            << tag
            << ".\n";
      }
      else {
        throw;
      }
    }
  }
  return h;
}
#endif /* art_Framework_Core_detail_getAssnsHandle_h */

// Local Variables:
// mode: c++
// End:
