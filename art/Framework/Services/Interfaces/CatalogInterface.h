#ifndef art_Framework_Services_Interfaces_CatalogInterface_h
#define art_Framework_Services_Interfaces_CatalogInterface_h

// -*- C++ -*-
//
// Package:     Services
// Class  :     CatalogInterface
//

/*

 Description: Abstract base class for services that return URI's when asked to get
      next file in a series, and deal with declarations that output files
      have been written.  We have in mind that SAMProtocol will inherit from
      this interface class.

*/
//
// Original Author:  Mark Fischler
//         Created:  Wed  25 Jul, 2012
//

#include "art/Framework/Services/Interfaces/FileDisposition.h"
#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Persistency/Common/fwd.h"
#include "fhiclcpp/fwd.h"

#include <string>
#include <vector>

namespace art {
  class CatalogInterface;

  class EventID;
}

class art::CatalogInterface {
public:
  void configure(std::vector<std::string> const & items);
  int  getNextFileURI(std::string & uri, double & waitTime);
  void updateStatus(std::string const & uri, FileDisposition status);
  void outputFileClosed(std::string const & module_label,
                        std::string const & fileFQname);
  void outputFileOpened(std::string const & module_label);
  void outputModuleInitiated(std::string const & module_label,
                             fhicl::ParameterSet const & pset);
  void eventSelected(std::string const & module_label,
                     EventID const & event_id,
                     HLTGlobalStatus const & acceptance_info);
  bool isSearchable();
  void rewind();
  virtual ~CatalogInterface() = default;
private:
  // Classes inheriting this interface must provide the following methods:
  virtual void doConfigure(std::vector<std::string> const & item) = 0;
  virtual int  doGetNextFileURI(std::string & uri, double & waitTime) = 0;
  virtual void doUpdateStatus(std::string const & uri, FileDisposition status) = 0;
  virtual void doOutputFileOpened(std::string const & module_label) = 0;
  virtual void doOutputModuleInitiated(std::string const & module_label,
                                       fhicl::ParameterSet const & pset) = 0;
  virtual void doOutputFileClosed(std::string const & module_label,
                                  std::string const & fileFQname) = 0;
  virtual void doEventSelected(std::string const & module_label,
                               EventID const & event_id,
                               HLTGlobalStatus const & acceptance_info) = 0;
  virtual bool doIsSearchable() = 0;
  virtual void doRewind() = 0;
};

inline void art::CatalogInterface::configure(std::vector<std::string> const & items)
{
  doConfigure(items);
}

inline int art::CatalogInterface::getNextFileURI(std::string & uri, double & waitTime)
{
  return doGetNextFileURI(uri, waitTime);
}

inline void art::CatalogInterface::updateStatus(std::string const & uri, FileDisposition status)
{
  doUpdateStatus(uri, status);
}

inline void art::CatalogInterface::outputFileClosed
(std::string const & module_label, std::string const & fileFQname)
{
  doOutputFileClosed(module_label, fileFQname);
}

inline void art::CatalogInterface::outputFileOpened
(std::string const & module_label)
{
  doOutputFileOpened(module_label);
}

inline void art::CatalogInterface::outputModuleInitiated
(std::string const & module_label,
 fhicl::ParameterSet const & pset)
{
  doOutputModuleInitiated(module_label, pset);
}

inline void art::CatalogInterface::eventSelected(std::string const & module_label,
    EventID const & event_id,
    HLTGlobalStatus const & acceptance_info)
{
  doEventSelected(module_label, event_id, acceptance_info);
}

inline bool art::CatalogInterface::isSearchable()
{
  return doIsSearchable();
}

inline void art::CatalogInterface::rewind()
{
  doRewind();
}

DECLARE_ART_SERVICE_INTERFACE(art::CatalogInterface,LEGACY)
#endif /* art_Framework_Services_Interfaces_CatalogInterface_h */

// Local Variables:
// mode: c++
// End:
