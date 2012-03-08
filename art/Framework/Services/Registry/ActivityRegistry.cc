//
// Package:     ServiceRegistry
// Class  :     ActivityRegistry
//

#define AR_IMPL
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#undef AR_IMPL

#include "cetlib/container_algorithms.h"
#include "cpp0x/algorithm"
#include "cpp0x/functional"

namespace {
  template<class T>
  static
  inline
  void
  copySlotsToFrom(T& iTo, T& iFrom)
  {
    typename T::slot_list_type slots = iFrom.slots();

    cet::for_all(slots, std::bind( &T::connect, iTo, std::placeholders::_1) );
  }

  template<class T>
  static
  inline
  void
  copySlotsToFromReverse(T& iTo, T& iFrom)
  {
    // This handles service slots that are supposed to be in reverse
    // order of construction. Copying new ones in is a little
    // tricky.  Here is an example of what follows
    // slots in iTo before  4 3 2 1  and copy in slots in iFrom 8 7 6 5
    // reverse both  1 2 3 4  plus 5 6 7 8
    // then do the copy 1 2 3 4 5 6 7 8
    // then reverse back again to get the desired order
    // 8 7 6 5 4 3 2 1

    typename T::slot_list_type slotsFrom = iFrom.slots();
    typename T::slot_list_type slotsTo   = iTo.slots();

    std::reverse(slotsTo.begin(), slotsTo.end());
    std::reverse(slotsFrom.begin(), slotsFrom.end());

    cet::for_all(slotsFrom, std::bind( &T::connect, iTo, std::placeholders::_1) );

    std::reverse(slotsTo.begin(), slotsTo.end());

    // Be nice and put these back in the state they were
    // at the beginning
    std::reverse(slotsFrom.begin(), slotsFrom.end());
  }

}

void
art::ActivityRegistry::connect(ActivityRegistry& iOther)
{
   sPostBeginJob_.connect(iOther.sPostBeginJob_);
   sPostEndJob_.connect(iOther.sPostEndJob_);

   sJobFailure_.connect(iOther.sJobFailure_);

   sPreSource_.connect(iOther.sPreSource_);
   sPostSource_.connect(iOther.sPostSource_);

   sPreSourceSubRun_.connect(iOther.sPreSourceSubRun_);
   sPostSourceSubRun_.connect(iOther.sPostSourceSubRun_);

   sPreSourceRun_.connect(iOther.sPreSourceRun_);
   sPostSourceRun_.connect(iOther.sPostSourceRun_);

   sPreOpenFile_.connect(iOther.sPreOpenFile_);
   sPostOpenFile_.connect(iOther.sPostOpenFile_);

   sPreCloseFile_.connect(iOther.sPreCloseFile_);
   sPostCloseFile_.connect(iOther.sPostCloseFile_);

   sPreProcessEvent_.connect(iOther.sPreProcessEvent_);
   sPostProcessEvent_.connect(iOther.sPostProcessEvent_);

   sPreBeginRun_.connect(iOther.sPreBeginRun_);
   sPostBeginRun_.connect(iOther.sPostBeginRun_);

   sPreEndRun_.connect(iOther.sPreEndRun_);
   sPostEndRun_.connect(iOther.sPostEndRun_);

   sPreBeginSubRun_.connect(iOther.sPreBeginSubRun_);
   sPostBeginSubRun_.connect(iOther.sPostBeginSubRun_);

   sPreEndSubRun_.connect(iOther.sPreEndSubRun_);
   sPostEndSubRun_.connect(iOther.sPostEndSubRun_);

   sPreProcessPath_.connect(iOther.sPreProcessPath_);
   sPostProcessPath_.connect(iOther.sPostProcessPath_);

   sPrePathBeginRun_.connect(iOther.sPrePathBeginRun_);
   sPostPathBeginRun_.connect(iOther.sPostPathBeginRun_);

   sPrePathEndRun_.connect(iOther.sPrePathEndRun_);
   sPostPathEndRun_.connect(iOther.sPostPathEndRun_);

   sPrePathBeginSubRun_.connect(iOther.sPrePathBeginSubRun_);
   sPostPathBeginSubRun_.connect(iOther.sPostPathBeginSubRun_);

   sPrePathEndSubRun_.connect(iOther.sPrePathEndSubRun_);
   sPostPathEndSubRun_.connect(iOther.sPostPathEndSubRun_);

   sPreModule_.connect(iOther.sPreModule_);
   sPostModule_.connect(iOther.sPostModule_);

   sPreModuleBeginRun_.connect(iOther.sPreModuleBeginRun_);
   sPostModuleBeginRun_.connect(iOther.sPostModuleBeginRun_);

   sPreModuleEndRun_.connect(iOther.sPreModuleEndRun_);
   sPostModuleEndRun_.connect(iOther.sPostModuleEndRun_);

   sPreModuleBeginSubRun_.connect(iOther.sPreModuleBeginSubRun_);
   sPostModuleBeginSubRun_.connect(iOther.sPostModuleBeginSubRun_);

   sPreModuleEndSubRun_.connect(iOther.sPreModuleEndSubRun_);
   sPostModuleEndSubRun_.connect(iOther.sPostModuleEndSubRun_);

   sPreModuleConstruction_.connect(iOther.sPreModuleConstruction_);
   sPostModuleConstruction_.connect(iOther.sPostModuleConstruction_);

   sPostBeginJobWorkers_.connect(iOther.sPostBeginJobWorkers_);

   sPreModuleBeginJob_.connect(iOther.sPreModuleBeginJob_);
   sPostModuleBeginJob_.connect(iOther.sPostModuleBeginJob_);

   sPreModuleEndJob_.connect(iOther.sPreModuleEndJob_);
   sPostModuleEndJob_.connect(iOther.sPostModuleEndJob_);

   sPostServiceReconfigure_.connect(iOther.sPostServiceReconfigure_);
}

void
art::ActivityRegistry::copySlotsFrom(ActivityRegistry& iOther)
{
  copySlotsToFrom(sPostBeginJob_,iOther.sPostBeginJob_);
  copySlotsToFromReverse(sPostEndJob_,iOther.sPostEndJob_);

  copySlotsToFromReverse(sJobFailure_,iOther.sJobFailure_);

  copySlotsToFrom(sPreSource_,iOther.sPreSource_);
  copySlotsToFromReverse(sPostSource_,iOther.sPostSource_);

  copySlotsToFrom(sPreSourceSubRun_,iOther.sPreSourceSubRun_);
  copySlotsToFromReverse(sPostSourceSubRun_,iOther.sPostSourceSubRun_);

  copySlotsToFrom(sPreSourceRun_,iOther.sPreSourceRun_);
  copySlotsToFromReverse(sPostSourceRun_,iOther.sPostSourceRun_);

  copySlotsToFrom(sPreOpenFile_,iOther.sPreOpenFile_);
  copySlotsToFromReverse(sPostOpenFile_,iOther.sPostOpenFile_);

  copySlotsToFrom(sPreCloseFile_,iOther.sPreCloseFile_);
  copySlotsToFromReverse(sPostCloseFile_,iOther.sPostCloseFile_);

  copySlotsToFrom(sPreProcessEvent_,iOther.sPreProcessEvent_);
  copySlotsToFromReverse(sPostProcessEvent_,iOther.sPostProcessEvent_);

  copySlotsToFrom(sPreBeginRun_,iOther.sPreBeginRun_);
  copySlotsToFromReverse(sPostBeginRun_,iOther.sPostBeginRun_);

  copySlotsToFrom(sPreEndRun_,iOther.sPreEndRun_);
  copySlotsToFromReverse(sPostEndRun_,iOther.sPostEndRun_);

  copySlotsToFrom(sPreBeginSubRun_,iOther.sPreBeginSubRun_);
  copySlotsToFromReverse(sPostBeginSubRun_,iOther.sPostBeginSubRun_);

  copySlotsToFrom(sPreEndSubRun_,iOther.sPreEndSubRun_);
  copySlotsToFromReverse(sPostEndSubRun_,iOther.sPostEndSubRun_);

  copySlotsToFrom(sPreProcessPath_,iOther.sPreProcessPath_);
  copySlotsToFromReverse(sPostProcessPath_,iOther.sPostProcessPath_);

  copySlotsToFrom(sPrePathBeginRun_,iOther.sPrePathBeginRun_);
  copySlotsToFromReverse(sPostPathBeginRun_,iOther.sPostPathBeginRun_);

  copySlotsToFrom(sPrePathEndRun_,iOther.sPrePathEndRun_);
  copySlotsToFromReverse(sPostPathEndRun_,iOther.sPostPathEndRun_);

  copySlotsToFrom(sPrePathBeginSubRun_,iOther.sPrePathBeginSubRun_);
  copySlotsToFromReverse(sPostPathBeginSubRun_,iOther.sPostPathBeginSubRun_);

  copySlotsToFrom(sPrePathEndSubRun_,iOther.sPrePathEndSubRun_);
  copySlotsToFromReverse(sPostPathEndSubRun_,iOther.sPostPathEndSubRun_);

  copySlotsToFrom(sPreModule_,iOther.sPreModule_);
  copySlotsToFromReverse(sPostModule_,iOther.sPostModule_);

  copySlotsToFrom(sPreModuleBeginRun_,iOther.sPreModuleBeginRun_);
  copySlotsToFromReverse(sPostModuleBeginRun_,iOther.sPostModuleBeginRun_);

  copySlotsToFrom(sPreModuleEndRun_,iOther.sPreModuleEndRun_);
  copySlotsToFromReverse(sPostModuleEndRun_,iOther.sPostModuleEndRun_);

  copySlotsToFrom(sPreModuleBeginSubRun_,iOther.sPreModuleBeginSubRun_);
  copySlotsToFromReverse(sPostModuleBeginSubRun_,iOther.sPostModuleBeginSubRun_);

  copySlotsToFrom(sPreModuleEndSubRun_,iOther.sPreModuleEndSubRun_);
  copySlotsToFromReverse(sPostModuleEndSubRun_,iOther.sPostModuleEndSubRun_);

  copySlotsToFrom(sPreModuleConstruction_,iOther.sPreModuleConstruction_);
  copySlotsToFromReverse(sPostModuleConstruction_,iOther.sPostModuleConstruction_);

  copySlotsToFrom(sPreModuleBeginJob_,iOther.sPreModuleBeginJob_);
  copySlotsToFromReverse(sPostModuleBeginJob_,iOther.sPostModuleBeginJob_);

  copySlotsToFrom(sPreModuleEndJob_,iOther.sPreModuleEndJob_);
  copySlotsToFromReverse(sPostModuleEndJob_,iOther.sPostModuleEndJob_);

  copySlotsToFromReverse(sPostBeginJobWorkers_,iOther.sPostBeginJobWorkers_);

  copySlotsToFrom(sPostServiceReconfigure_,iOther.sPostServiceReconfigure_);
}
