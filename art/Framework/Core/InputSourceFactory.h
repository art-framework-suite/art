#ifndef art_Framework_Core_InputSourceFactory_h
#define art_Framework_Core_InputSourceFactory_h

// ======================================================================
//
// InputSourceFactory
//
// ======================================================================

#include "art/Framework/Core/InputSource.h"
#include "art/Utilities/PluginSuffixes.h"
#include "cetlib/LibraryManager.h"
#include "fhiclcpp/ParameterSet.h"

#include <memory>

namespace art {
  class InputSourceFactory;
}

// ----------------------------------------------------------------------

class art::InputSourceFactory {
  InputSourceFactory(InputSourceFactory const&) = delete;
  InputSourceFactory& operator=(InputSourceFactory const&) = delete;

public:
  static std::unique_ptr<InputSource> make(fhicl::ParameterSet const&,
                                           InputSourceDescription&);

private:
  explicit InputSourceFactory() = default;
  static InputSourceFactory& instance();

  cet::LibraryManager lm_{Suffixes::source()};

}; // InputSourceFactory

// ======================================================================

#endif /* art_Framework_Core_InputSourceFactory_h */

// Local Variables:
// mode: c++
// End:
