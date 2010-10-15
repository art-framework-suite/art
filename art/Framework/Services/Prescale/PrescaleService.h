#ifndef FWCore_PrescaleService_PrescaleService_h
#define FWCore_PrescaleService_PrescaleService_h


#include "art/Persistency/Provenance/EventID.h"

#include "art/Framework/Core/EventProcessor.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/Utilities/Exception.h"

#include "boost/thread/mutex.hpp"
#include "fhiclcpp/ParameterSet.h"

#include <map>
#include <string>
#include <vector>


namespace art {
  namespace service {

    class PrescaleService
    {
    public:
      //
      // construction/destruction
      //
      PrescaleService(const fhicl::ParameterSet&,ActivityRegistry&) throw (artZ::Exception);
      ~PrescaleService();


      //
      // member functions
      //

      void reconfigure(const fhicl::ParameterSet &);

      unsigned int getPrescale(unsigned int lvl1Index,
                               const std::string&prescaledPath)throw(artZ::Exception);
      unsigned int getPrescale(const std::string&prescaledPath)throw(artZ::Exception);


      void postBeginJob() {;}
      void postEndJob() {;}
      void preEventProcessing(const art::EventID&, const art::Timestamp&) {;}
      void postEventProcessing(const art::Event&) {;}
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
