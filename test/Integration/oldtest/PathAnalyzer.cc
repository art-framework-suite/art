#include <algorithm>
#include <iterator>
#include <sstream>

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "messagefacility/MessageLogger/MessageLogger.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Utilities/Algorithms.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/ParameterSetRegistry.h"

namespace arttest
{
  class PathAnalyzer : public art::EDAnalyzer
  {
  public:

    explicit PathAnalyzer(fhicl::ParameterSet const&);
    virtual ~PathAnalyzer();

    virtual void analyze(art::Event const&, art::EventSetup const&);
    virtual void beginJob(art::EventSetup const&);
    virtual void endJob();

  private:
    void dumpTriggerNamesServiceInfo(char const* where);
  }; // class PathAnalyzer

  //--------------------------------------------------------------------
  //
  // Implementation details

  PathAnalyzer::PathAnalyzer(fhicl::ParameterSet const&) { }

  PathAnalyzer::~PathAnalyzer() {}

  void
  PathAnalyzer::analyze(art::Event const&, art::EventSetup const&)
  {
    dumpTriggerNamesServiceInfo("analyze");
  }

  void
  PathAnalyzer::beginJob(art::EventSetup const&)
  {
    dumpTriggerNamesServiceInfo("beginJob");

    // Make sure we can get a the process parameter set. This test
    // doesn't really belong here, but I had to stick it somewhere
    // quickly...

    fhicl::ParameterSet ppset = art::getProcessParameterSet();
    assert (ppset.id().isValid());
  }

  void
  PathAnalyzer::endJob()
  {
    dumpTriggerNamesServiceInfo("endJob");
  }

  void
  PathAnalyzer::dumpTriggerNamesServiceInfo(char const* where)
  {
    typedef art::ServiceHandle<art::TriggerNamesService>  TNS;
    typedef std::vector<std::string> stringvec;

    TNS tns;
    std::ostringstream message;

    stringvec const& trigpaths = tns->getTrigPaths();
    message << "dumpTriggernamesServiceInfo called from PathAnalyzer::"
	    << where << '\n';
    message << "trigger paths are: ";

    art::copy_all(trigpaths, std::ostream_iterator<std::string>(message, " "));
    message << '\n';

    for (stringvec::const_iterator i = trigpaths.begin(), e = trigpaths.end();
	 i != e;
	 ++i)
      {
	message << "path name: " << *i << " contains: ";
	art::copy_all(tns->getTrigPathModules(*i), std::ostream_iterator<std::string>(message, " "));
	message << '\n';
      }

    message << "trigger ParameterSet:\n"
	    << tns->getTriggerPSet()
	    << '\n';

    mf::LogInfo("PathAnalyzer") << "TNS size: " << tns->size()
				 << "\n"
				 << message.str()
				 << std::endl;
  }

} // arttest

using arttest::PathAnalyzer;
DEFINE_ART_MODULE(PathAnalyzer);
