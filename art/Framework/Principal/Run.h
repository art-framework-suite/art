#ifndef art_Framework_Principal_Run_h
#define art_Framework_Principal_Run_h
// vim: set sw=2 expandtab :

//
//  This is the primary interface for accessing per run EDProducts
//  and inserting new derived products.
//
//  For its usage, see "art/Framework/Principal/DataViewImpl.h"
//

#include "art/Framework/Principal/DataViewImpl.h"
#include "art/Framework/Principal/fwd.h"
#include "canvas/Persistency/Common/Wrapper.h"
#include "canvas/Persistency/Provenance/ProductToken.h"
#include "canvas/Persistency/Provenance/RunAuxiliary.h"
#include "canvas/Persistency/Provenance/RunID.h"
#include "canvas/Utilities/TypeID.h"

#include <memory>
#include <string>
#include <utility>

namespace art {

class Run final : public DataViewImpl {

public: // MEMBER FUNCTIONS -- Special Member Functions

  ~Run();

  explicit
  Run(RunPrincipal&, ModuleDescription const&, RangeSet const& rs = RangeSet::invalid());

  Run(Run const&) = delete;

  Run(Run&&) = delete;

  Run&
  operator=(Run const&) = delete;

  Run&
  operator=(Run&&) = delete;

public: // MEMBER FUNCTIONS -- User-facing API

  RunID const
  id() const;

};

} // namespace art

#endif /* art_Framework_Principal_Run_h */

// Local Variables:
// mode: c++
// End:
