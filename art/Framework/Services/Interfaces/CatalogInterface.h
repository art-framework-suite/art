#ifndef art_Framework_Services_UserInteraction_CatalogInterface_h
#define art_Framework_Services_UserInteraction_CatalogInterface_h

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

#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Provenance/EventID.h"
#include "art/Persistency/Common/HLTGlobalStatus.h"
#include <string>

namespace art {
  class CatalogInterface;
}

namespace art {
  typedef std::string URI;
  typedef std::string module_id_t;
}

class art::CatalogInterface
{
public:
  int  getNextFileURI (URI & uri, double & waitTime);
  void updateStatus (URI const & uri, int status);
  void outputFileClosed (module_id_t const & module_id, URI const & file);
  void outputFileOpened (module_id_t const & module_id);
  void outputModuleInitiated (module_id_t const & module_id,
                              fhicl::ParameterSet const & pset);
  void eventSelected(module_id_t const & module_id,
                     EventID event_id,
                     HLTGlobalStatus acceptance_info);
  virtual ~CatalogInterface() = default;
private:
  // Classes inheriting this interface must provide the following methods:
  virtual int  doGetNextFileURI(URI & uri, double & waitTime) = 0;
  virtual void doUpdateStatus (URI const & uri, int status) = 0;
  virtual void doOutputFileOpened (module_id_t const & module_id) = 0;
  virtual void doOutputModuleInitiated (module_id_t const & module_id,
                                        fhicl::ParameterSet const & pset ) = 0;
  virtual void doOutputFileClosed (module_id_t const & module_id,
                                   URI const & file) = 0;
  virtual void doEventSelected(module_id_t const & module_id,
                     EventID event_id, HLTGlobalStatus acceptance_info) = 0;
};

inline int  art::CatalogInterface::getNextFileURI (URI & uri, double & waitTime) {
  return doGetNextFileURI (uri, waitTime);
}
inline void art::CatalogInterface::updateStatus (URI const & uri, int status) {
  doUpdateStatus (uri, status);
}
inline void art::CatalogInterface::outputFileClosed
  (module_id_t const & module_id, URI const & file) {
  doOutputFileClosed (module_id, file);
}
inline void art::CatalogInterface::outputFileOpened
                          (module_id_t const & module_id){
  doOutputFileOpened (module_id);
}
inline void art::CatalogInterface::outputModuleInitiated
                     (module_id_t const & module_id,
          fhicl::ParameterSet const & pset) {
  doOutputModuleInitiated(module_id, pset);
}
inline void art::CatalogInterface::eventSelected(module_id_t const & module_id,
               EventID event_id,
               HLTGlobalStatus acceptance_info) {
  doEventSelected (module_id, event_id, acceptance_info);
}

#endif /* art_Framework_Services_UserInteraction_CatalogInterface_h */

// Local Variables:
// mode: c++
// End:
