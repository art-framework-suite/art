#ifndef art_Framework_Core_ProductMetaData_h
#define art_Framework_Core_ProductMetaData_h
////////////////////////////////////////////////////////////////////////
// ProductMetaData
//
// Singleton-like class to provide const access to the
// MasterProductRegistry.
////////////////////////////////////////////////////////////////////////
#include "cetlib/exempt_ptr.h"

namespace art {
  class ProductMetaData;

  class EventProcessor;
  class MasterProductRegistry;
}

class art::ProductMetaData {
 public:
  static MasterProductRegistry const &get();

  friend class EventProcessor;

 private:
  ProductMetaData();

  static cet::exempt_ptr<MasterProductRegistry const> mpr_;
};

#endif /* art_Framework_Core_ProductMetaData_h */

// Local Variables:
// mode: c++
// End:
