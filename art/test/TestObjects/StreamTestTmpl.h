#ifndef test_TestObjects_StreamTestTmpl_h
#define test_TestObjects_StreamTestTmpl_h

#include <vector>
#include "art/test/TestObjects/StreamTestSimple.h"

namespace arttestprod
{
  template <class T>
    struct Ord
    {
      T data_;
    };

  template <class T> //, class U = Ord<T> >
    struct StreamTestTmpl
    {
      T data_;
      //U more_;
    };

  typedef Ord<Simple> OSimple;
  //typedef Simple OSimple;
}

#endif /* test_TestObjects_StreamTestTmpl_h */

// Local Variables:
// mode: c++
// End:
