#ifndef art_Persistency_Provenance_Transient_h
#define art_Persistency_Provenance_Transient_h
// -*- C++ -*-
//
// Package:     Provenance
// Class  :     Transient
//
/**\class Transient Transient.h DataFormats/Provenance/interface/Transient.h

 Description: ROOT safe bool

 Usage:
    We define a template for transients  in order to guarantee that value_
    is always reset when ever a new instance of this class is read from a file.

*/
//
// Original Author:  Bill Tanenbaum
//         Created:  Sat Aug 18 17:30:08 EDT 2007
//
//

// system include files

// user include files

// forward declarations
namespace art {

template <typename T>
class Transient {
public:
  typedef T value_type;
  Transient(T value = T()) : value_(value) {}
  operator T() const { return value_; }
  // We cannot ref-qualify the assignment operator because of GCC_XML.
  Transient & operator=(T rh) { value_ = rh; return *this; }
  T const& get() const { return value_;}
  T & get() { return value_;}
private:
  T value_;
};
}
#endif /* art_Persistency_Provenance_Transient_h */

// Local Variables:
// mode: c++
// End:
