#ifndef art_Framework_Core_InputSourceFactory_h
#define art_Framework_Core_InputSourceFactory_h
// vim: set sw=2 expandtab :

#include <memory>

namespace fhicl {
  class ParameterSet;
} // namespace fhicl

namespace art {

  class InputSource;
  struct InputSourceDescription;

  namespace InputSourceFactory {

    std::unique_ptr<InputSource> make(fhicl::ParameterSet const&,
                                      InputSourceDescription&);

  } // namespace InputSourceFactory

} // namespace art

#endif /* art_Framework_Core_InputSourceFactory_h */

// Local Variables:
// mode: c++
// End:
