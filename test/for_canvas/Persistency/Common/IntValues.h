#ifndef test_Persistency_Common_IntValues_h
#define test_Persistency_Common_IntValues_h
// -*- C++ -*-
//
// Package:     Common
// Class  :     IntValues
//
/**\class IntValues IntValues.h DataFormats/Common/interface/IntValues.h

 Description: Classes used for simple tests
 Usage:
    <usage>

*/
//
// Original Author:  Chris Jones
//         Created:  Wed Oct 31 16:23:41 EDT 2007
//
//

// system include files

// user include files

// forward declarations

namespace test_with_reflex {
  struct IntValue {
    int value_;
    IntValue(int iValue): value_(iValue) {}
  };

  struct IntValue2 :public IntValue {
    IntValue2(int iValue2):IntValue(iValue2) {}
  };
}

#endif /* test_Persistency_Common_IntValues_h */

// Local Variables:
// mode: c++
// End:
