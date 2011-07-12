// change
#include "art/Framework/IO/Root/GetFileFormatVersion.h"

////////////////////////////////////////////////////////////////////////
// Version history
////////////////////////////////////
//
// In general:
//
// * A change to the persistent format which is both backward- and
// forawrd-compatible requires no change to the version or era.
//
// * A backward-compatible change should change the version.
//
// * A non-backward-compatible change should change the era,
//
// Era / Version   Comments
//
//    -       11   As forked from CMS.
// ART_2011a   1   Initial ART-only format
// ART_2011a   2   No FileID in ROOT file.
// ART_2011a   3   Bunch crossing, orbit number and store number removed
//                 from EventAuxiliary.
// ART_2011a   4   PtrVector member indicies_-> indices_.
//
////////////////////////////////////////////////////////////////////////

namespace art
{
  int getFileFormatVersion() { return 4; }
}
