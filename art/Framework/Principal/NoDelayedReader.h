#ifndef art_Framework_Principal_NoDelayedReader_h
#define art_Framework_Principal_NoDelayedReader_h
// vim: set sw=2 expandtab :

#include "art/Framework/Principal/DelayedReader.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/fwd.h"

#include <memory>

namespace art {

  class NoDelayedReader : public DelayedReader {
  public:
    ~NoDelayedReader() noexcept;
    NoDelayedReader();

  private:
    [[noreturn]] std::unique_ptr<EDProduct> getProduct_(Group const*,
                                                        ProductID,
                                                        RangeSet&) const;
  };

} // namespace art

#endif /* art_Framework_Principal_NoDelayedReader_h */

// Local Variables:
// mode: c++
// End:
