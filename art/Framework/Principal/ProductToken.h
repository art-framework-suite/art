#ifndef art_Framework_Principal_ProductToken_h
#define art_Framework_Principal_ProductToken_h

//==============================================================
// ProductToken is used to enable efficient product lookup via a
// consumes statement given in a module's constructor (e.g.):
//
//   ProductToken<int> nPotsToken_{consumes<int>{inputTag_}};
//   ...
//   auto const& nPotsH = e.getByToken(nPotsToken_); => Handle<int>;
//
// It is an all-private class that only ConsumesRecorder and
// DataViewImpl can access.
// ==============================================================

#include "canvas/Utilities/InputTag.h"

namespace art {

  class ConsumesRecorder;
  class DataViewImpl;

  template <typename T>
  class ProductToken {
    friend class ConsumesRecorder;
    friend class DataViewImpl;

    auto const& inputTag() const { return inputTag_; }
    explicit ProductToken() = default;
    explicit ProductToken(InputTag const& t) : inputTag_{t} {}
    InputTag inputTag_{};
  };

}

#endif
