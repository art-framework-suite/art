#ifndef TestObjects_Thing_h
#define TestObjects_Thing_h

namespace arttest {

  struct Thing {
    ~Thing() { }
    Thing():a() { }
    int a;
  };

}

#endif
