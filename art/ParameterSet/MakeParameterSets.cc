#include "art/ParameterSet/MakeParameterSets.h"

#include "art/ParameterSet/PythonProcessDesc.h"


namespace edm {

  boost::shared_ptr<ProcessDesc>
  readConfig(const std::string & config)
  {
    PythonProcessDesc pythonProcessDesc(config);
    return pythonProcessDesc.processDesc();
  }

  boost::shared_ptr<edm::ProcessDesc>
  readConfig(const std::string & config, int argc, char * argv[])
  {
    PythonProcessDesc pythonProcessDesc(config, argc, argv);
    return pythonProcessDesc.processDesc();
  }


  void
  makeParameterSets(std::string const& configtext,
                  boost::shared_ptr<fhicl::ParameterSet>& main,
                  boost::shared_ptr<std::vector<fhicl::ParameterSet> >& serviceparams)
  {
    PythonProcessDesc pythonProcessDesc(configtext);
    boost::shared_ptr<edm::ProcessDesc> processDesc = pythonProcessDesc.processDesc();
    main = processDesc->getProcessPSet();
    serviceparams = processDesc->getServicesPSets();
  }

} // namespace edm
