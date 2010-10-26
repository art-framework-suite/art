#ifndef DataFormats_Common_CopyPolicy_h
#define DataFormats_Common_CopyPolicy_h

namespace art {
  template<typename T>
  struct CopyPolicy{
    static const T & clone(const T & t) {
      return t;
    }
  };
}

#endif
