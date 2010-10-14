////////////////////////////////////////////////////////////////////////////////
//
// PrescaleService
// ---------------
//
//            04/25/2008 Philipp Schieferdecker <philipp.schieferdecker@cern.ch>
////////////////////////////////////////////////////////////////////////////////


#include "art/Framework/Services/Prescale/PrescaleService.h"

#include "art/ParameterSet/Registry.h"
#include "art/Framework/Core/Event.h"

#include "MessageFacility/MessageLogger.h"

#include <set>
#include <algorithm>


using namespace std;
using fhicl::ParameterSet;


namespace edm {
  namespace service {

    ////////////////////////////////////////////////////////////////////////////////
    // construction/destruction
    ////////////////////////////////////////////////////////////////////////////////

    //______________________________________________________________________________
    PrescaleService::PrescaleService(const ParameterSet& iPS,ActivityRegistry&iReg)
      throw (cms::Exception)
      : nLvl1Index_(0)
      , iLvl1IndexDefault_(0)
    {
      reconfigure(iPS);

      iReg.watchPostBeginJob(this,&PrescaleService::postBeginJob);
      iReg.watchPostEndJob(this,&PrescaleService::postEndJob);

      iReg.watchPreProcessEvent(this,&PrescaleService::preEventProcessing);
      iReg.watchPostProcessEvent(this,&PrescaleService::postEventProcessing);

      iReg.watchPreModule(this,&PrescaleService::preModule);
      iReg.watchPostModule(this,&PrescaleService::postModule);
    }


    //______________________________________________________________________________
    PrescaleService::~PrescaleService()
    {

    }


    ////////////////////////////////////////////////////////////////////////////////
    // implementation of member functions
    ////////////////////////////////////////////////////////////////////////////////

    //______________________________________________________________________________
    void PrescaleService::reconfigure(const ParameterSet &iPS)
    {

      ParameterSet prcPS = getProcessParameterSet();

      // find all HLTPrescaler modules
      set<string> prescalerModules;
      vector<string> allModules=prcPS.get<std::vector<std::string> >("@all_modules");
      for(unsigned int i=0;i<allModules.size();i++) {
        ParameterSet pset  = prcPS.get<fhicl::ParameterSet>(allModules[i]);
        string moduleLabel = pset.get<std::string>("@module_label");
        string moduleType  = pset.get<std::string>("@module_type");
        if (moduleType=="HLTPrescaler") prescalerModules.insert(moduleLabel);
      }

      // find all paths with an HLTPrescaler and check for <=1
      std::set<string> prescaledPathSet;
      vector<string> allPaths = prcPS.get<std::vector<std::string> >("@paths");
      for (unsigned int iP=0;iP<allPaths.size();iP++) {
        string pathName = allPaths[iP];
        vector<string> modules = prcPS.get<std::vector<std::string> >(pathName);
        for (unsigned int iM=0;iM<modules.size();iM++) {
          string moduleLabel = modules[iM];
          if (prescalerModules.erase(moduleLabel)>0) {
            set<string>::const_iterator itPath=prescaledPathSet.find(pathName);
            if (itPath==prescaledPathSet.end())
              prescaledPathSet.insert(pathName);
            else throw cms::Exception("DuplicatePrescaler")
              <<"path '"<<pathName<<"' has more than one HLTPrescaler!";
          }
        }
      }

      // get prescale table and check consistency with above information
      lvl1Labels_ = iPS.get<std::vector<std::string> >("lvl1Labels");
      nLvl1Index_ = lvl1Labels_.size();

      string lvl1DefaultLabel=
        iPS.get<std::string>("lvl1DefaultLabel","");
      for (unsigned int i=0;i<lvl1Labels_.size();i++)
        if (lvl1Labels_[i]==lvl1DefaultLabel) iLvl1IndexDefault_=i;

      vector<ParameterSet> vpsetPrescales=
        iPS.get<std::vector<fhicl::ParameterSet> >("prescaleTable");

      vector<string> prescaledPaths;
      for (unsigned int iVPSet=0;iVPSet<vpsetPrescales.size();iVPSet++) {
        ParameterSet psetPrescales = vpsetPrescales[iVPSet];
        string pathName = psetPrescales.get<std::string>("pathName");
        if (prescaledPathSet.erase(pathName)>0) {
          vector<unsigned int> prescales =
            psetPrescales.get<std::vector<unsigned int> >("prescales");
          if (prescales.size()!=nLvl1Index_) {
            throw cms::Exception("PrescaleTableMismatch")
              <<"path '"<<pathName<<"' has unexpected number of prescales";
          }
          prescaleTable_[pathName] = prescales;
        }
        else {
          throw cms::Exception("PrescaleTableUnknownPath")
            <<"path '"<<pathName<<"' is invalid or does not "
            <<"contain any HLTPrescaler";
        }
      }
    }

    //______________________________________________________________________________
    unsigned int PrescaleService::getPrescale(const std::string& prescaledPath)
      throw (cms::Exception)
    {
      return getPrescale(iLvl1IndexDefault_, prescaledPath);
    }

    //______________________________________________________________________________
    unsigned int PrescaleService::getPrescale(unsigned int lvl1Index,
                                              const std::string& prescaledPath)
      throw (cms::Exception)
    {
      if (lvl1Index>=nLvl1Index_)
        throw cms::Exception("InvalidLvl1Index")
          <<"lvl1Index '"<<lvl1Index<<"' exceeds number of prescale columns";

      boost::mutex::scoped_lock scoped_lock(mutex_);
      PrescaleTable_t::const_iterator it = prescaleTable_.find(prescaledPath);
      return (it==prescaleTable_.end()) ? 1 : it->second[lvl1Index];
    }


  } // namespace service
} // namespace edm
