#ifndef art_Framework_Core_MergeFilter_h
#define art_Framework_Core_MergeFilter_h

#include "art/Framework/Core/EDFilter.h"
#include "art/Framework/Core/MergeHelper.h"

namespace art {
  template <class T>
  class MergeFilter;
}

template <class T>
class art::MergeFilter : public art::EDFilter {
public:
  typedef T MergeDetail;
  explicit MergeFilter(fhicl::ParameterSet const &p);

  virtual bool filter(art::Event &e);

private:
  MergeHelper helper_;
  MergeDetail detail_;
};

template <class T>
art::MergeFilter::MergeFilter(fhicl::ParameterSet const &p)
  :
  EDFilter(),
  helper_(*this),
  detail_(p, helper_),
{
}

template <class T>
bool
art::MergeFilter::filter(art::Event &e) {
  // 1. Call detail.startEvent() if it exists.
  // 2. Ask detail object how many events to read.
  // 3. For each product to be merged:
  //   1. Read products and fill a container.
  //   2. Call merge function.
  //   3. Place resultant product in event.
  // 4. Call detail.finalizeEvent() if it exists.
}
#endif /* art_Framework_Core_MergeFilter_h */

// Local Variables:
// mode: c++
// End:
