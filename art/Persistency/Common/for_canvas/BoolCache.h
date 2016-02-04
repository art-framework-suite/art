#ifndef art_Persistency_Common_BoolCache_h
#define art_Persistency_Common_BoolCache_h
// -*- C++ -*-
//
// Package:     Common
// Class  :     BoolCache
//
/**\class BoolCache BoolCache.h DataFormats/Common/interface/BoolCache.h

 Description: ROOT safe cache flag

 Usage:
    We define an external TStreamer for this class in order to guarantee that isCached_
    is always reset to false when ever a new instance of this class is read from a file

*/
//
// Original Author:  Chris Jones
//         Created:  Sat Aug 18 17:30:08 EDT 2007
//
//

// system include files

// user include files

// forward declarations
namespace art {
class BoolCache {
public:
  BoolCache() : isCached_(false) {}
  BoolCache(bool iValue) : isCached_(iValue) {}
  operator bool() { return isCached_; }
  // Cannot ref-qualify assignment operator because of GCC_XML.
  BoolCache & operator=( bool b ) { isCached_ = b; return *this; }
private:
  bool isCached_;
};

}
#endif /* art_Persistency_Common_BoolCache_h */

// Local Variables:
// mode: c++
// End:
