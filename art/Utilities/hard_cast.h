#ifndef art_Utilities_hard_cast_h
#define art_Utilities_hard_cast_h

#include <cstring>

namespace art {
  // For use when only a C++ {dynamic,static,reinterpret}_cast is not
  // sufficient to the task. The only case of this known currently is
  // when using dlopen, dlsym, etc. and a void * must be cast to a
  // function pointer.
  template <typename PTR>
  PTR
  hard_cast(void * src);
  template <typename PTR>
  void
  hard_cast(void * src, PTR & dest);
}

template <typename PTR>
inline
PTR
art::hard_cast(void * src)
{
  PTR dest;
  hard_cast(src, dest);
  return dest;
}

template <typename PTR>
inline
void
art::hard_cast(void * src, PTR & dest)
{
  memcpy(&dest, &src, sizeof(PTR));
}
#endif /* art_Utilities_hard_cast_h */

// Local Variables:
// mode: c++
// End:
