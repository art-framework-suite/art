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
   postBeginJobSignal_.connect(iOther.postBeginJobSignal_);
   postEndJobSignal_.connect(iOther.postEndJobSignal_);

   jobFailureSignal_.connect(iOther.jobFailureSignal_);

   preSourceSignal_.connect(iOther.preSourceSignal_);
   postSourceSignal_.connect(iOther.postSourceSignal_);

   preSourceSubRunSignal_.connect(iOther.preSourceSubRunSignal_);
   postSourceSubRunSignal_.connect(iOther.postSourceSubRunSignal_);

   preSourceRunSignal_.connect(iOther.preSourceRunSignal_);
   postSourceRunSignal_.connect(iOther.postSourceRunSignal_);

   preOpenFileSignal_.connect(iOther.preOpenFileSignal_);
   postOpenFileSignal_.connect(iOther.postOpenFileSignal_);

   preCloseFileSignal_.connect(iOther.preCloseFileSignal_);
   postCloseFileSignal_.connect(iOther.postCloseFileSignal_);

   preProcessEventSignal_.connect(iOther.preProcessEventSignal_);
   postProcessEventSignal_.connect(iOther.postProcessEventSignal_);

   preBeginRunSignal_.connect(iOther.preBeginRunSignal_);
   postBeginRunSignal_.connect(iOther.postBeginRunSignal_);

   preEndRunSignal_.connect(iOther.preEndRunSignal_);
   postEndRunSignal_.connect(iOther.postEndRunSignal_);

   preBeginSubRunSignal_.connect(iOther.preBeginSubRunSignal_);
   postBeginSubRunSignal_.connect(iOther.postBeginSubRunSignal_);

   preEndSubRunSignal_.connect(iOther.preEndSubRunSignal_);
   postEndSubRunSignal_.connect(iOther.postEndSubRunSignal_);

   preProcessPathSignal_.connect(iOther.preProcessPathSignal_);
   postProcessPathSignal_.connect(iOther.postProcessPathSignal_);

   prePathBeginRunSignal_.connect(iOther.prePathBeginRunSignal_);
   postPathBeginRunSignal_.connect(iOther.postPathBeginRunSignal_);

   prePathEndRunSignal_.connect(iOther.prePathEndRunSignal_);
   postPathEndRunSignal_.connect(iOther.postPathEndRunSignal_);

   prePathBeginSubRunSignal_.connect(iOther.prePathBeginSubRunSignal_);
   postPathBeginSubRunSignal_.connect(iOther.postPathBeginSubRunSignal_);

   prePathEndSubRunSignal_.connect(iOther.prePathEndSubRunSignal_);
   postPathEndSubRunSignal_.connect(iOther.postPathEndSubRunSignal_);

   preModuleSignal_.connect(iOther.preModuleSignal_);
   postModuleSignal_.connect(iOther.postModuleSignal_);

   preModuleBeginRunSignal_.connect(iOther.preModuleBeginRunSignal_);
   postModuleBeginRunSignal_.connect(iOther.postModuleBeginRunSignal_);

   preModuleEndRunSignal_.connect(iOther.preModuleEndRunSignal_);
   postModuleEndRunSignal_.connect(iOther.postModuleEndRunSignal_);

   preModuleBeginSubRunSignal_.connect(iOther.preModuleBeginSubRunSignal_);
   postModuleBeginSubRunSignal_.connect(iOther.postModuleBeginSubRunSignal_);

   preModuleEndSubRunSignal_.connect(iOther.preModuleEndSubRunSignal_);
   postModuleEndSubRunSignal_.connect(iOther.postModuleEndSubRunSignal_);

   preModuleConstructionSignal_.connect(iOther.preModuleConstructionSignal_);
   postModuleConstructionSignal_.connect(iOther.postModuleConstructionSignal_);

   postBeginJobWorkersSignal_.connect(iOther.postBeginJobWorkersSignal_);

   preModuleBeginJobSignal_.connect(iOther.preModuleBeginJobSignal_);
   postModuleBeginJobSignal_.connect(iOther.postModuleBeginJobSignal_);

   preModuleEndJobSignal_.connect(iOther.preModuleEndJobSignal_);
   postModuleEndJobSignal_.connect(iOther.postModuleEndJobSignal_);
}

void
art::ActivityRegistry::copySlotsFrom(ActivityRegistry& iOther)
{
  copySlotsToFrom(postBeginJobSignal_,iOther.postBeginJobSignal_);
  copySlotsToFromReverse(postEndJobSignal_,iOther.postEndJobSignal_);

  copySlotsToFromReverse(jobFailureSignal_,iOther.jobFailureSignal_);

  copySlotsToFrom(preSourceSignal_,iOther.preSourceSignal_);
  copySlotsToFromReverse(postSourceSignal_,iOther.postSourceSignal_);

  copySlotsToFrom(preSourceSubRunSignal_,iOther.preSourceSubRunSignal_);
  copySlotsToFromReverse(postSourceSubRunSignal_,iOther.postSourceSubRunSignal_);

  copySlotsToFrom(preSourceRunSignal_,iOther.preSourceRunSignal_);
  copySlotsToFromReverse(postSourceRunSignal_,iOther.postSourceRunSignal_);

  copySlotsToFrom(preOpenFileSignal_,iOther.preOpenFileSignal_);
  copySlotsToFromReverse(postOpenFileSignal_,iOther.postOpenFileSignal_);

  copySlotsToFrom(preCloseFileSignal_,iOther.preCloseFileSignal_);
  copySlotsToFromReverse(postCloseFileSignal_,iOther.postCloseFileSignal_);

  copySlotsToFrom(preProcessEventSignal_,iOther.preProcessEventSignal_);
  copySlotsToFromReverse(postProcessEventSignal_,iOther.postProcessEventSignal_);

  copySlotsToFrom(preBeginRunSignal_,iOther.preBeginRunSignal_);
  copySlotsToFromReverse(postBeginRunSignal_,iOther.postBeginRunSignal_);

  copySlotsToFrom(preEndRunSignal_,iOther.preEndRunSignal_);
  copySlotsToFromReverse(postEndRunSignal_,iOther.postEndRunSignal_);

  copySlotsToFrom(preBeginSubRunSignal_,iOther.preBeginSubRunSignal_);
  copySlotsToFromReverse(postBeginSubRunSignal_,iOther.postBeginSubRunSignal_);

  copySlotsToFrom(preEndSubRunSignal_,iOther.preEndSubRunSignal_);
  copySlotsToFromReverse(postEndSubRunSignal_,iOther.postEndSubRunSignal_);

  copySlotsToFrom(preProcessPathSignal_,iOther.preProcessPathSignal_);
  copySlotsToFromReverse(postProcessPathSignal_,iOther.postProcessPathSignal_);

  copySlotsToFrom(prePathBeginRunSignal_,iOther.prePathBeginRunSignal_);
  copySlotsToFromReverse(postPathBeginRunSignal_,iOther.postPathBeginRunSignal_);

  copySlotsToFrom(prePathEndRunSignal_,iOther.prePathEndRunSignal_);
  copySlotsToFromReverse(postPathEndRunSignal_,iOther.postPathEndRunSignal_);

  copySlotsToFrom(prePathBeginSubRunSignal_,iOther.prePathBeginSubRunSignal_);
  copySlotsToFromReverse(postPathBeginSubRunSignal_,iOther.postPathBeginSubRunSignal_);

  copySlotsToFrom(prePathEndSubRunSignal_,iOther.prePathEndSubRunSignal_);
  copySlotsToFromReverse(postPathEndSubRunSignal_,iOther.postPathEndSubRunSignal_);

  copySlotsToFrom(preModuleSignal_,iOther.preModuleSignal_);
  copySlotsToFromReverse(postModuleSignal_,iOther.postModuleSignal_);

  copySlotsToFrom(preModuleBeginRunSignal_,iOther.preModuleBeginRunSignal_);
  copySlotsToFromReverse(postModuleBeginRunSignal_,iOther.postModuleBeginRunSignal_);

  copySlotsToFrom(preModuleEndRunSignal_,iOther.preModuleEndRunSignal_);
  copySlotsToFromReverse(postModuleEndRunSignal_,iOther.postModuleEndRunSignal_);

  copySlotsToFrom(preModuleBeginSubRunSignal_,iOther.preModuleBeginSubRunSignal_);
  copySlotsToFromReverse(postModuleBeginSubRunSignal_,iOther.postModuleBeginSubRunSignal_);

  copySlotsToFrom(preModuleEndSubRunSignal_,iOther.preModuleEndSubRunSignal_);
  copySlotsToFromReverse(postModuleEndSubRunSignal_,iOther.postModuleEndSubRunSignal_);

  copySlotsToFrom(preModuleConstructionSignal_,iOther.preModuleConstructionSignal_);
  copySlotsToFromReverse(postModuleConstructionSignal_,iOther.postModuleConstructionSignal_);

  copySlotsToFrom(preModuleBeginJobSignal_,iOther.preModuleBeginJobSignal_);
  copySlotsToFromReverse(postModuleBeginJobSignal_,iOther.postModuleBeginJobSignal_);

  copySlotsToFrom(preModuleEndJobSignal_,iOther.preModuleEndJobSignal_);
  copySlotsToFromReverse(postModuleEndJobSignal_,iOther.postModuleEndJobSignal_);

  copySlotsToFromReverse(postBeginJobWorkersSignal_,iOther.postBeginJobWorkersSignal_);
}
