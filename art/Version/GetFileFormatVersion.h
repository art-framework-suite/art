#ifndef art_Version_GetFileFormatVersion_h
#define art_Version_GetFileFormatVersion_h

namespace art
{
  // We do not inline this function to help avoid inconsistent
  // versions being inlined into different libraries.

  int getFileFormatVersion();
}
#endif /* art_Version_GetFileFormatVersion_h */

// Local Variables:
// mode: c++
// End:
