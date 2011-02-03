#ifndef test_TestObjects_StreamTestSimple_h
#define test_TestObjects_StreamTestSimple_h

#include "art/Persistency/Common/SortedCollection.h"

namespace arttestprod
{
  struct Simple
  {
    typedef int key_type;
    int key_;
    double data_;

    key_type  id() const { return key_; }
    bool operator==(const Simple&) const { return true; }
    bool operator<(const Simple&) const { return true; }
  };

  typedef art::SortedCollection<Simple> StreamTestSimple;

  struct X0123456789012345678901234567890123456789012345678901234567890123456789012345678901
  {
    int blob_;
  };

  typedef X0123456789012345678901234567890123456789012345678901234567890123456789012345678901 Pig;
}

#endif /* test_TestObjects_StreamTestSimple_h */

// Local Variables:
// mode: c++
// End:
