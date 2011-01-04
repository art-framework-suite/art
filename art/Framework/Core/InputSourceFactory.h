#ifndef FWCore_Framework_InputSourceFactory_h
#define FWCOre_Framework_InputSourceFactory_h

// ======================================================================
//
// InputSourceFactory
//
// ======================================================================

#include "art/Framework/Core/InputSource.h"
#include "art/Framework/Core/LibraryManager.h"

#include "fhiclcpp/ParameterSet.h"
#include <memory>
#include <string>

// ----------------------------------------------------------------------

namespace art {
  class InputSourceFactory;
}

class art::InputSourceFactory
{
  // non-copyable:
  InputSourceFactory( InputSourceFactory const & );
  void  operator = ( InputSourceFactory const & );

 public:
  static std::auto_ptr<InputSource>
     makeInputSource(fhicl::ParameterSet const&,
                     InputSourceDescription const&);

private:
  LibraryManager lm_;

  InputSourceFactory();
  ~InputSourceFactory();

  static InputSourceFactory &
     the_factory_();

};  // InputSourceFactory


// ======================================================================

#endif
