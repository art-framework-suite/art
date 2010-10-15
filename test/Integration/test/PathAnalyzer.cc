#include <algorithm>
#include <iterator>
#include <sstream>

#include "art/Framework/Core/EDAnalyzer.h"
#include "art/Framework/Core/MakerMacros.h"
#include "art/Framework/Core/TriggerNamesService.h"
#include "art/MessageLogger/MessageLogger.h"
#include "art/Framework/Services/Registry/Service.h"
#include "art/Utilities/Algorithms.h"
#include "art/ParameterSet/ParameterSet.h"
#include "art/ParameterSet/Registry.h"

namespace edmtest
{
  class PathAnalyzer : public art::EDAnalyzer
  {
  public:

    explicit PathAnalyzer(art::ParameterSet const&);
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

  PathAnalyzer::PathAnalyzer(art::ParameterSet const&) { }

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

    art::ParameterSet ppset = art::getProcessParameterSet();
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
    typedef art::Service<art::service::TriggerNamesService>  TNS;
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

    art::LogInfo("PathAnalyzer") << "TNS size: " << tns->size()
				 << "\n"
				 << message.str()
				 << std::endl;
  }

} // namespace edmtest

using edmtest::PathAnalyzer;
DEFINE_FWK_MODULE(PathAnalyzer);
