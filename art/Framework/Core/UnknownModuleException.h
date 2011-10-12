#ifndef art_Framework_Core_UnknownModuleException_h
#define art_Framework_Core_UnknownModuleException_h

// ======================================================================
//
// UnknownModuleException: Exception thrown when trying to instance a
//                         module which is not registered to the system
//
// ======================================================================

#include "cetlib/exception.h"

namespace art {

  class UnknownModuleException
    : public cet::exception
  {
  public:
    UnknownModuleException(const std::string & moduletype):
      cet::exception("UnknownModule")
    {
      (*this) << "Module " << moduletype << " was not registered \n"
        "Perhaps your module type is misspelled or is not a framework plugin\n"
        "Try running EdmPluginDump to obtain a list of available Plugins\n";
    }

   // use compiler-generated copy c'tor and copy assignment

    ~UnknownModuleException() throw(){}

  }; // UnknownModuleException

} // edm

#endif /* art_Framework_Core_UnknownModuleException_h */

// Local Variables:
// mode: c++
// End:
