#ifndef art_Framework_Principal_NoDelayedReader_h
#define art_Framework_Principal_NoDelayedReader_h
// vim: set sw=2 expandtab :

#include "art/Persistency/Common/DelayedReader.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/EDProduct.h"
#include "canvas/Persistency/Provenance/ProvenanceFwd.h"

#include <memory>

namespace art {

class BranchKey;
class RangeSet;
class TypeID;

class NoDelayedReader : public DelayedReader {

public:

  ~NoDelayedReader() noexcept;
  NoDelayedReader();

private:

  [[noreturn]]
  std::unique_ptr<EDProduct>
  getProduct_(BranchKey const&, TypeID const&, RangeSet&) const;

};

} // namespace art

#endif /* art_Framework_Principal_NoDelayedReader_h */

// Local Variables:
// mode: c++
// End:
