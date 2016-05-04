#ifndef art_test_TestObjects_Thing_h
#define art_test_TestObjects_Thing_h

namespace arttest {

  struct Thing {
    ~Thing() { }
    Thing():a() { }
    int a;
  };

}

#endif /* art_test_TestObjects_Thing_h */

// Local Variables:
// mode: c++
// End:
