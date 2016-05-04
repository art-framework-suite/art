#ifndef art_test_TestObjects_StreamTestThing_h
#define art_test_TestObjects_StreamTestThing_h

#include <vector>

namespace arttestprod {

  struct StreamTestThing
  {
    ~StreamTestThing();
    explicit StreamTestThing(int sz);
    StreamTestThing();

    std::vector<int> data_;
  };

}

#endif /* art_test_TestObjects_StreamTestThing_h */

// Local Variables:
// mode: c++
// End:
