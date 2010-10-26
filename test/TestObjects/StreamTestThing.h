#ifndef TestObjects_StreamTestThing_h
#define TestObjects_StreamTestThing_h

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

#endif
