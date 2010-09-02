#ifndef Modules_IterateNTimesLooper_h
#define Modules_IterateNTimesLooper_h

//
// Package:     Modules
// Class  :     IterateNTimesLooper
//
/**\class IterateNTimesLooper IterateNTimesLooper.h FWCore/Modules/interface/IterateNTimesLooper.h

*/


// user include files
#include "art/Framework/Core/EDLooper.h"


class IterateNTimesLooper : public edm::EDLooper
{

   public:
      IterateNTimesLooper(const edm::ParameterSet& );
      virtual ~IterateNTimesLooper();

      // ---------- const member functions ---------------------

      // ---------- static member functions --------------------

      // ---------- member functions ---------------------------
      virtual void startingNewLoop(unsigned int ) ;
      virtual Status duringLoop(const edm::Event&) ;
      virtual Status endOfLoop(unsigned int) ;

   private:
      IterateNTimesLooper(const IterateNTimesLooper&); // stop default

      const IterateNTimesLooper& operator=(const IterateNTimesLooper&); // stop default

      // ---------- member data --------------------------------
      unsigned int max_;
      unsigned int times_;
      bool shouldStop_;
};


#endif  // Modules_IterateNTimesLooper_h
