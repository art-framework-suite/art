#ifndef UpdaterService_H
#define UpdaterService_H


#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "fhiclcpp/ParameterSet.h"

#include <map>
#include <string>


namespace edm {
  class EventID;
  class TimeStamp;
}


class UpdaterService {
public:
  UpdaterService(const fhicl::ParameterSet & cfg, edm::ActivityRegistry & r );
  ~UpdaterService();

  void init(const edm::EventID&, const edm::Timestamp&); //preEvent
  //  void initModule(const edm::ModuleDescription&); //premodule

  bool checkOnce(std::string);
  bool check(std::string, std::string);

private:
  void theInit();
  std::map< std::string, uint > theCounts;
  const edm::EventID * theEventId;
};

#endif  // UpdaterService_H
