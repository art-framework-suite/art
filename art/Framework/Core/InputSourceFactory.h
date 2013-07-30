#ifndef art_Framework_Core_InputSourceFactory_h
#define art_Framework_Core_InputSourceFactory_h

// ======================================================================
//
// InputSourceFactory
//
// ======================================================================

#include "art/Framework/Core/InputSource.h"
#include "art/Utilities/LibraryManager.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include <string>

namespace art {
  class InputSourceFactory;
}

// ----------------------------------------------------------------------

class art::InputSourceFactory
{
  InputSourceFactory( InputSourceFactory const & ) = delete;
  InputSourceFactory& operator = ( InputSourceFactory const & ) = delete;

 public:
  static std::unique_ptr<InputSource>
     make(fhicl::ParameterSet const&,
          InputSourceDescription &);

private:
  LibraryManager lm_;

  InputSourceFactory();
  ~InputSourceFactory();

  static InputSourceFactory &
     the_factory_();

};  // InputSourceFactory


// ======================================================================

#endif /* art_Framework_Core_InputSourceFactory_h */

// Local Variables:
// mode: c++
// End:
