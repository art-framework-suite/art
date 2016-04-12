#ifndef test_TestObjects_Thing_h
#define test_TestObjects_Thing_h

namespace arttest {

  struct Thing {
    ~Thing() { }
    Thing():a() { }
    int a;
  };

}

#endif /* test_TestObjects_Thing_h */

// Local Variables:
// mode: c++
// End:
