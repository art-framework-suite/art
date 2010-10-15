#ifndef DataFormats_Common_ClonePolicy_h
#define DataFormats_Common_ClonePolicy_h

namespace art {
  template<typename T>
  struct ClonePolicy{
    static T * clone(const T & t) {
      return t.clone();
    }
  };
}

#endif
