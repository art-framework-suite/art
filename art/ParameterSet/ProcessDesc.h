#error "Using obsolete ParameterSet/ProcessDesc.h"


#ifndef ParameterSet_ProcessDesc_h
#define ParameterSet_ProcessDesc_h


#include "boost/shared_ptr.hpp"
#include "fhiclcpp/ParameterSet.h"
#include <vector>


namespace art
{

  class ProcessDesc
  {

  public:
    explicit ProcessDesc(const fhicl::ParameterSet & pset);

    // construct from the configuration language string
    explicit ProcessDesc(const std::string& config);

    ~ProcessDesc();

    // get the ParameterSet that describes the process
    boost::shared_ptr<fhicl::ParameterSet> getProcessPSet() const;

    // get the dependencies for this module
    /** the return string is a list of comma-separated
      * names of the modules on which modulename depends*/
    std::string  getDependencies(const std::string& modulename);

    // get the descriptions of the services
    boost::shared_ptr<std::vector<fhicl::ParameterSet> > getServicesPSets() const;

    void addService(const fhicl::ParameterSet & pset);
    // add a service as an empty pset
    void addService(const std::string & service);
    // add a service if it's not already there
    void addDefaultService(const std::string & service);
    // add some defaults services, and some forced
    void addServices(std::vector<std::string> const& defaultServices,
                     std::vector<std::string> const& forcedServices);

    void setRegistry() const;

  private:
    typedef std::vector<std::string> Strs;
    //Path and sequence information
    boost::shared_ptr<fhicl::ParameterSet> pset_;
    boost::shared_ptr<std::vector< fhicl::ParameterSet> > services_;
  };

}  // namespace art

#endif  // ParameterSet_ProcessDesc_h
