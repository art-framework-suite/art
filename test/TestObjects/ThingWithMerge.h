#ifndef test_TestObjects_ThingWithMerge_h
#define test_TestObjects_ThingWithMerge_h

namespace arttest {

  struct ThingWithMerge {
    ~ThingWithMerge() { }
    ThingWithMerge(): a() { }
    bool mergeProduct(ThingWithMerge const & newThing);
    int a;
  };

}

#endif /* test_TestObjects_ThingWithMerge_h */

// Local Variables:
// mode: c++
// End:
