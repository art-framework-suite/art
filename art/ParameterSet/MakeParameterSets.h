#error "Using obsolete ParameterSet/MakeParameterSets.h"


#ifndef FWCoreParameterSet_MakeParameterSets_h
#define FWCoreParameterSet_MakeParameterSets_h


//----------------------------------------------------------------------
//
// Declare functions used to create ParameterSets.
//
//----------------------------------------------------------------------


#include "art/ParameterSet/ProcessDesc.h"
#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"
#include <string>
#include <vector>


namespace art {

  // input can either be a python file name or a python config string
  boost::shared_ptr<art::ProcessDesc>
  readConfig(const std::string & config);

  // same, but with arguments
  boost::shared_ptr<art::ProcessDesc>
  readConfig(const std::string & config, int argc, char * argv[]);

  // essentially the same as the previous method
  void
  makeParameterSets(std::string const& configtext,
                  boost::shared_ptr<fhicl::ParameterSet>& main,
                  boost::shared_ptr<std::vector<fhicl::ParameterSet> >& serviceparams);

  // deprecated
  boost::shared_ptr<art::ProcessDesc>
  readConfigFile(const std::string & fileName) {return readConfig(fileName);}

} // namespace art

#endif  // FWCoreParameterSet_MakeParameterSets_h
