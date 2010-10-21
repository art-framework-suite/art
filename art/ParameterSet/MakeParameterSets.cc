#include "art/ParameterSet/MakeParameterSets.h"

//#include "art/ParameterSet/PythonProcessDesc.h"


namespace art {

  boost::shared_ptr<ProcessDesc>
  readConfig(const std::string & config)
  {
    // TODO:  replace this body
    #if 0
    PythonProcessDesc pythonProcessDesc(config);
    return pythonProcessDesc.processDesc();
    #endif  // 0
  }

  boost::shared_ptr<art::ProcessDesc>
  readConfig(const std::string & config, int argc, char * argv[])
  {
    // TODO:  replace this body
    #if 0
    PythonProcessDesc pythonProcessDesc(config, argc, argv);
    return pythonProcessDesc.processDesc();
    #endif  // 0
  }


  void
  makeParameterSets(std::string const& configtext,
                  boost::shared_ptr<fhicl::ParameterSet>& main,
                  boost::shared_ptr<std::vector<fhicl::ParameterSet> >& serviceparams)
  {
    // TODO:  replace this body
    #if 0
    PythonProcessDesc pythonProcessDesc(configtext);
    boost::shared_ptr<art::ProcessDesc> processDesc = pythonProcessDesc.processDesc();
    main = processDesc->getProcessPSet();
    serviceparams = processDesc->getServicesPSets();
    #endif  // 0
  }

} // namespace art
