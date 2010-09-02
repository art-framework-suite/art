#ifndef FWCore_Services_Tracer_h
#define FWCore_Services_Tracer_h

//
// Package:     Services
// Class  :     Tracer
//
/**\class Tracer Tracer.h FWCore/Services/interface/Tracer.h

*/


// forward declarations

#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/ParameterSet/ParameterSet.h"


namespace edm {
   namespace service {
      class Tracer {
public:
         Tracer(const ParameterSet&,ActivityRegistry&);

         void postBeginJob();
         void postEndJob();

         void preBeginRun(RunID const& id, Timestamp const& ts);
         void postBeginRun(Run const& run);

         void preBeginLumi(SubRunID const& id, Timestamp const& ts);
         void postBeginLumi(SubRun const& run);

         void preEvent(EventID const& id, Timestamp const& ts);
         void postEvent(Event const& ev);

         void preEndLumi(SubRunID const& id, Timestamp const& ts);
         void postEndLumi(SubRun const& run);

         void preEndRun(RunID const& id, Timestamp const& ts);
         void postEndRun(Run const& run);

         void preSourceConstruction(ModuleDescription const& md);
         void postSourceConstruction(ModuleDescription const& md);

         void preModuleConstruction(ModuleDescription const& md);
         void postModuleConstruction(ModuleDescription const& md);

         void preModuleBeginJob(ModuleDescription const& md);
         void postModuleBeginJob(ModuleDescription const& md);

         void preModuleBeginRun(ModuleDescription const& md);
         void postModuleBeginRun(ModuleDescription const& md);

         void preModuleBeginLumi(ModuleDescription const& md);
         void postModuleBeginLumi(ModuleDescription const& md);

         void preModuleEvent(ModuleDescription const& md);
         void postModuleEvent(ModuleDescription const& md);

         void preModuleEndLumi(ModuleDescription const& md);
         void postModuleEndLumi(ModuleDescription const& md);

         void preModuleEndRun(ModuleDescription const& md);
         void postModuleEndRun(ModuleDescription const& md);

         void preModuleEndJob(ModuleDescription const& md);
         void postModuleEndJob(ModuleDescription const& md);

         void preSourceEvent();
         void postSourceEvent();

         void preSourceLumi();
         void postSourceLumi();

         void preSourceRun();
         void postSourceRun();

         void preOpenFile();
         void postOpenFile();

         void preCloseFile();
         void postCloseFile();

         void prePathBeginRun(std::string const& s);
         void postPathBeginRun(std::string const& s, HLTPathStatus const& hlt);

         void prePathBeginLumi(std::string const& s);
         void postPathBeginLumi(std::string const& s, HLTPathStatus const& hlt);

         void prePathEvent(std::string const& s);
         void postPathEvent(std::string const& s, HLTPathStatus const& hlt);

         void prePathEndLumi(std::string const& s);
         void postPathEndLumi(std::string const& s, HLTPathStatus const& hlt);

         void prePathEndRun(std::string const& s);
         void postPathEndRun(std::string const& s, HLTPathStatus const& hlt);

private:
         std::string indention_;
         unsigned int depth_;

      };
   }
}

#endif  // FWCore_Services_Tracer_h
