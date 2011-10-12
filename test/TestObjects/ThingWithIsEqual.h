#ifndef test_TestObjects_ThingWithIsEqual_h
#define test_TestObjects_ThingWithIsEqual_h

namespace arttest {

  struct ThingWithIsEqual {
    ~ThingWithIsEqual() { }
    ThingWithIsEqual(): a() { }
    bool isProductEqual(ThingWithIsEqual const & thingWithIsEqual) const;
    int a;
  };

}

#endif /* test_TestObjects_ThingWithIsEqual_h */

// Local Variables:
// mode: c++
// End:
