#ifndef FWCore_PrescaleService_PrescaleService_h
#define FWCore_PrescaleService_PrescaleService_h


#include "art/Persistency/Provenance/EventID.h"

#include "art/Framework/Core/EventProcessor.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/Utilities/Exception.h"


#include "boost/thread/mutex.hpp"


#include <string>
#include <vector>
#include <map>


namespace edm {
  namespace service {

    class PrescaleService
    {
    public:
      //
      // construction/destruction
      //
      PrescaleService(const ParameterSet&,ActivityRegistry&) throw (cms::Exception);
      ~PrescaleService();


      //
      // member functions
      //

      void reconfigure(const ParameterSet &);

      unsigned int getPrescale(unsigned int lvl1Index,
			       const std::string&prescaledPath)throw(cms::Exception);
      unsigned int getPrescale(const std::string&prescaledPath)throw(cms::Exception);


      void postBeginJob() {;}
      void postEndJob() {;}
      void preEventProcessing(const edm::EventID&, const edm::Timestamp&) {;}
      void postEventProcessing(const edm::Event&) {;}
      void preModule(const ModuleDescription&) {;}
      void postModule(const ModuleDescription&) {;}


    private:
      //
      // private member functions
      //


      //
      // member data
      //
      typedef std::vector<std::string>                         VString_t;
      typedef std::map<std::string,std::vector<unsigned int> > PrescaleTable_t;

      boost::mutex    mutex_;
      unsigned int    nLvl1Index_;
      unsigned int    iLvl1IndexDefault_;
      VString_t       lvl1Labels_;
      PrescaleTable_t prescaleTable_;
    };
  }
}

#endif
