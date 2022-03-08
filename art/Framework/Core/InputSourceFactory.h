#ifndef art_Framework_Core_InputSourceFactory_h
#define art_Framework_Core_InputSourceFactory_h
// vim: set sw=2 expandtab :

#include "art/Framework/Core/fwd.h"
#include "fhiclcpp/fwd.h"

#include <memory>

namespace art::InputSourceFactory {
  std::unique_ptr<InputSource> make(fhicl::ParameterSet const&,
                                    InputSourceDescription&);
}

#endif /* art_Framework_Core_InputSourceFactory_h */

// Local Variables:
// mode: c++
// End:
