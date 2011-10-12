#ifndef test_TestObjects_StreamTestThing_h
#define test_TestObjects_StreamTestThing_h

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

#endif /* test_TestObjects_StreamTestThing_h */

// Local Variables:
// mode: c++
// End:
