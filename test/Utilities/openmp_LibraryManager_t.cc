#include <iostream>

#include "art/Framework/Core/RootDictionaryManager.h"
#include "art/Utilities/LibraryManager.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "fhiclcpp/ParameterSet.h"

int main(int argc, char **argv)
{
  if (argc < 3) {
    return 1;
  }

  mf::MessageDrop::instance()->jobMode = std::string("analysis");
  mf::MessageDrop::instance()->runEvent = std::string("JobSetup");
  mf::StartMessageFacility(mf::MessageFacilityService::MultiThread,
                           fhicl::ParameterSet());
  mf::LogInfo("MF_INIT_OK") << "Messagelogger initialization complete.";

  //   art::RootDictionaryManager rdm;
  //  art::LibraryManager lm_source("source");
  //  lm_source.loadAllLibraries();
  //  art::LibraryManager lm_service("service");
  // lm_service.loadAllLibraries();
  art::LibraryManager lm_module("module");
  // lm_module.loadAllLibraries();

  typedef size_t(*testFunc_t)(size_t, size_t);
  testFunc_t tf = lm_module.getSymbolByLibspec<testFunc_t>(argv[1], argv[2]);
  size_t total = tf(10, 20);
  std::cout << "Total: " << total << std::endl;
}
