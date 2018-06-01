#ifndef art_Utilities_forceRootDynamicPathRescan_h
#define art_Utilities_forceRootDynamicPathRescan_h
////////////////////////////////////////////////////////////////////////
// forceRootDynamicPathRescan()
//
// This function should be called whenever it is necessary to tell Root
// to re-obtain its cached DynamicPath from the environment. This may be
// necessary in an art suite (say) plugin constructor which relies on
// Root finding dictionariesin a directory which is not added to
// (DY)LD_LIBRARY_PATH until after Root is initialized.
//
// N.B. This does not introduce a Root link dependence to
// libart_Utilities, only to the first library calling this function
// (however indirectly) from a compile unit.
//
// 2018-05-29 CHG.
////////////////////////////////////////////////////////////////////////
#include "TROOT.h"
#include "TSystem.h"

namespace art {
  void forceRootDynamicPathRescan();
}

inline
void art::forceRootDynamicPathRescan()
{
  // FIXME: Disabled pending Root investigations of issue with
  // gInterpreter->RescanLibraryMap().
  //
  // if (ROOT::Internal::gROOTLocal) {
  //   gSystem->SetDynamicPath(nullptr);
  //   gInterpreter->RescanLibraryMap();
  // }
}
#endif /* art_Utilities_forceRootDynamicPathRescan_h */

// Local Variables:
// mode: c++
// End:
