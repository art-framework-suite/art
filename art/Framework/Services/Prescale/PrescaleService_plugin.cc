// ======================================================================
//
// PrescaleService
//
// ======================================================================


#include "art/Framework/Core/Event.h"
#include "art/Framework/Core/EventProcessor.h"
#include "art/Framework/Core/TriggerReport.h"
#include "art/Framework/Services/Registry/ServiceMaker.h"
#include "art/ParameterSet/processParameterSetID.h"
#include "art/Persistency/Provenance/EventID.h"
#include "cetlib/exception.h"
#include "boost/thread/mutex.hpp"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>


namespace art {
  class PrescaleService;
}


using namespace art;
using namespace fhicl;
using namespace std;


// ----------------------------------------------------------------------


class art::PrescaleService
{
public:
  // construction/destruction
  PrescaleService(ParameterSet const &,ActivityRegistry & );
  ~PrescaleService();

  // member functions

  void reconfigure(ParameterSet const & );

  unsigned int getPrescale(unsigned int lvl1Index,
                           string const & prescaledPath);
  unsigned int getPrescale(string const & prescaledPath);

  void postBeginJob() { }
  void postEndJob() { }
  void preEventProcessing(EventID const &, const Timestamp & ) { }
  void postEventProcessing(Event const & ) { }
  void preModule(ModuleDescription const & ) { }
  void postModule(ModuleDescription const & ) { }

private:
  // member data
  typedef vector<string>                         VString_t;
  typedef map<string,vector<unsigned int> > PrescaleTable_t;

  boost::mutex    mutex_;
  unsigned int    nLvl1Index_;
  unsigned int    iLvl1IndexDefault_;
  VString_t       lvl1Labels_;
  PrescaleTable_t prescaleTable_;
};  // PrescaleService


////////////////////////////////////////////////////////////////////////////
// construction/destruction
////////////////////////////////////////////////////////////////////////////

//__________________________________________________________________________
PrescaleService::PrescaleService(ParameterSet const & iPS,ActivityRegistry & iReg)
  : nLvl1Index_(0)
  , iLvl1IndexDefault_(0)
{
  reconfigure(iPS);

  iReg.watchPostBeginJob(this, & PrescaleService::postBeginJob);
  iReg.watchPostEndJob(this, & PrescaleService::postEndJob);

  iReg.watchPreProcessEvent(this, & PrescaleService::preEventProcessing);
  iReg.watchPostProcessEvent(this, & PrescaleService::postEventProcessing);

  iReg.watchPreModule(this, & PrescaleService::preModule);
  iReg.watchPostModule(this, & PrescaleService::postModule);
}


//__________________________________________________________________________
PrescaleService::~PrescaleService()
{ }


////////////////////////////////////////////////////////////////////////////
// implementation of member functions
////////////////////////////////////////////////////////////////////////////

//__________________________________________________________________________
void PrescaleService::reconfigure(ParameterSet const & iPS)
{
  ParameterSet const &
    prcPS = ParameterSetRegistry::get( processParameterSetID() );

  // find all HLTPrescaler modules
  set<string> prescalerModules;
  VString_t allModules=prcPS.get<VString_t>("@all_modules");
  for(unsigned int i=0;i<allModules.size();i++) {
    ParameterSet pset  = prcPS.get<ParameterSet>(allModules[i]);
    string moduleLabel = pset.get<string>("@module_label");
    string moduleType  = pset.get<string>("@module_type");
    if (moduleType=="HLTPrescaler") prescalerModules.insert(moduleLabel);
  }

  // find all paths with an HLTPrescaler and check for <=1
  set<string> prescaledPathSet;
  VString_t allPaths = prcPS.get<VString_t>("@paths");
  for (unsigned int iP=0;iP<allPaths.size();iP++) {
    string pathName = allPaths[iP];
    VString_t modules = prcPS.get<VString_t>(pathName);
    for (unsigned int iM=0;iM<modules.size();iM++) {
      string moduleLabel = modules[iM];
      if (prescalerModules.erase(moduleLabel)>0) {
        set<string>::const_iterator itPath=prescaledPathSet.find(pathName);
        if (itPath==prescaledPathSet.end())
          prescaledPathSet.insert(pathName);
        else
          throw cet::exception("DuplicatePrescaler")
            << "path '" << pathName << "' has more than one HLTPrescaler!";
      }
    }
  }

  // get prescale table and check consistency with above information
  lvl1Labels_ = iPS.get<VString_t>("lvl1Labels");
  nLvl1Index_ = lvl1Labels_.size();

  string lvl1DefaultLabel=
    iPS.get<string>("lvl1DefaultLabel","");
  for (unsigned int i=0;i<lvl1Labels_.size();i++)
    if (lvl1Labels_[i]==lvl1DefaultLabel) iLvl1IndexDefault_=i;

  vector<ParameterSet> vpsetPrescales=
    iPS.get<vector<ParameterSet> >("prescaleTable");

  VString_t prescaledPaths;
  for (unsigned int iVPSet=0;iVPSet<vpsetPrescales.size();iVPSet++) {
    ParameterSet psetPrescales = vpsetPrescales[iVPSet];
    string pathName = psetPrescales.get<string>("pathName");
    if (prescaledPathSet.erase(pathName)>0) {
      vector<unsigned int> prescales =
        psetPrescales.get<vector<unsigned int> >("prescales");
      if (prescales.size()!=nLvl1Index_)
        throw cet::exception("PrescaleTableMismatch")
          << "path '" << pathName << "' has unexpected number of prescales";
      prescaleTable_[pathName] = prescales;
    }
    else
      throw cet::exception("PrescaleTableUnknownPath")
        << "path '" << pathName
        << "' is invalid or does not contain any HLTPrescaler";
  }
}

//__________________________________________________________________________
unsigned int PrescaleService::getPrescale(string const & prescaledPath)
{
  return getPrescale(iLvl1IndexDefault_, prescaledPath);
}

//__________________________________________________________________________
unsigned int PrescaleService::getPrescale(unsigned int lvl1Index,
                                          string const & prescaledPath)
{
  if (lvl1Index>=nLvl1Index_)
    throw cet::exception("InvalidLvl1Index")
      << "lvl1Index '" << lvl1Index
      << "' exceeds number of prescale columns";

  boost::mutex::scoped_lock scoped_lock(mutex_);
  PrescaleTable_t::const_iterator it = prescaleTable_.find(prescaledPath);
  return (it==prescaleTable_.end()) ? 1 : it->second[lvl1Index];
}


// ----------------------------------------------------------------------


DEFINE_FWK_SERVICE(PrescaleService);


// ======================================================================
