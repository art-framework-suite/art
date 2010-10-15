#ifndef TestObjects_ThingWithIsEqual_h
#define TestObjects_ThingWithIsEqual_h

namespace arttest {

  struct ThingWithIsEqual {
    ~ThingWithIsEqual() { }
    ThingWithIsEqual():a() { }
    bool isProductEqual(ThingWithIsEqual const& thingWithIsEqual) const;
    int a;
  };

}

#endif
