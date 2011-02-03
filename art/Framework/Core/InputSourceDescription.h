#ifndef art_Framework_Core_InputSourceDescription_h
#define art_Framework_Core_InputSourceDescription_h

/*----------------------------------------------------------------------

InputSourceDescription : the stuff that is needed to configure an
input source that does not come in through the ParameterSet


----------------------------------------------------------------------*/

#include "art/Persistency/Provenance/ModuleDescription.h"
#include "boost/shared_ptr.hpp"
#include <string>

namespace art {
  class ProductRegistry;
  class ActivityRegistry;

  struct InputSourceDescription {
    InputSourceDescription() : moduleDescription_(), productRegistry_(0), actReg_(), maxEvents_(-1), maxSubRuns_(-1) {}
    InputSourceDescription(ModuleDescription const& md,
			   ProductRegistry& preg,
			   boost::shared_ptr<ActivityRegistry> areg,
			   int maxEvents,
			   int maxSubRuns) :
      moduleDescription_(md),
      productRegistry_(&preg),
      actReg_(areg),
      maxEvents_(maxEvents),
      maxSubRuns_(maxSubRuns)

    {}

    ModuleDescription moduleDescription_;
    ProductRegistry * productRegistry_;
    boost::shared_ptr<ActivityRegistry> actReg_;
    int maxEvents_;
    int maxSubRuns_;
  };

}  // art

#endif /* art_Framework_Core_InputSourceDescription_h */

// Local Variables:
// mode: c++
// End:
