#ifndef art_Utilities_DebugMacros_h
#define art_Utilities_DebugMacros_h

namespace art {
  struct debugvalue {

    debugvalue();

    int operator()() { return value_; }

    const char* cvalue_;
    int value_;
  };

extern debugvalue debugit;
}

#define FDEBUG(lev) if(lev <= debugit()) std::cerr

#endif /* art_Utilities_DebugMacros_h */

// Local Variables:
// mode: c++
// End:
