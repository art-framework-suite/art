// change
#include "art/Framework/IO/Root/GetFileFormatVersion.h"

////////////////////////////////////////////////////////////////////////
// Version history
////////////////////////////////////
//
// Era / Version   Comments
//
//    -       11   As forked from CMS.
// ART_2011a   1   Initial ART-only format
// ART_2011a   2   No FileID in ROOT file.
// ART_2011a   3   Bunch crossing, orbit number and store number removed
//                 from EventAuxiliary.
//
////////////////////////////////////////////////////////////////////////

namespace art
{
  int getFileFormatVersion() { return 3; }
}
