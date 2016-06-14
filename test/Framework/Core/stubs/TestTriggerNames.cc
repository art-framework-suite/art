
#include "art/Framework/Core/EDAnalyzer.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Persistency/Common/TriggerResults.h"

#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/System/TriggerNamesService.h"
#include "art/Framework/Core/TriggerNames.h"
#include "fhiclcpp/ParameterSetRegistry.h"

#include "art/Persistency/Provenance/ParameterSetID.h"
#include "art/Framework/Principal/Provenance.h"

#include "art/Framework/Core/ModuleMacros.h"

#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

namespace art {
  class EventSetup;
}

using namespace art;

namespace arttest
{

  class TestTriggerNames : public art::EDAnalyzer
  {
  public:

    typedef std::vector<std::string> Strings;

    explicit TestTriggerNames(fhicl::ParameterSet const&);
    virtual ~TestTriggerNames();

    virtual void analyze(art::Event const& e, art::EventSetup const& c);
    void endJob();

  private:

    bool status_;

    Strings expected_trigger_paths_;
    Strings expected_trigger_previous_;
    Strings expected_end_paths_;
    bool streamerSource_;
    bool dumpPSetRegistry_;

    art::TriggerNames triggerNamesI_;
  };

  // -----------------------------------------------------------------

  TestTriggerNames::TestTriggerNames(fhicl::ParameterSet const& ps):
    status_(true),
    expected_trigger_paths_(ps.get<Strings>("trigPaths", Strings())),
    expected_trigger_previous_(ps.get<Strings>("trigPathsPrevious", Strings())),
    expected_end_paths_(ps.get<Strings>("endPaths", Strings())),
    streamerSource_(ps.get<bool>("streamerSource", false)),
    dumpPSetRegistry_(ps.get<bool>("dumpPSetRegistry", false))
  {
  }

  // -----------------------------------------------------------------

  TestTriggerNames::~TestTriggerNames()
  {
  }

  // -----------------------------------------------------------------

  void TestTriggerNames::analyze(art::Event const& e,art::EventSetup const&)
  {
    if (dumpPSetRegistry_) {
      pset::Registry* psetRegistry = pset::Registry::instance();
      psetRegistry->print(std::cout);
    }

    // Runs some tests on the TriggerNamesService
    if (expected_trigger_paths_.size() > 0) {

      Strings triggernames;
      art::ServiceHandle<art::TriggerNamesService> tns;
      triggernames = tns->getTrigPaths();
      if (triggernames.size() != expected_trigger_paths_.size()) {
        std::cerr << "TestTriggerNames: "
             << "Expected and actual trigger path list not the same size" << std::endl;
        abort();
      }
      for (Strings::size_type i = 0; i < expected_trigger_paths_.size(); ++i) {
        if (triggernames[i] != expected_trigger_paths_[i]) {
          std::cerr << "TestTriggerNames: "
               << "Expected and actual trigger paths don't match" << std::endl;
          abort();
        }
      }
    }

    if (expected_end_paths_.size() > 0) {

      Strings endnames;
      art::ServiceHandle<art::TriggerNamesService> tns;
      endnames = tns->getEndPaths();
      if (endnames.size() != expected_end_paths_.size()) {
        std::cerr << "TestTriggerNames: "
             << "Expected and actual end path list not the same size" << std::endl;
        abort();
      }
      for (Strings::size_type i = 0; i < expected_end_paths_.size(); ++i) {
        if (endnames[i] != expected_end_paths_[i]) {
          std::cerr << "TestTriggerNames: "
               << "Expected and actual end paths don't match" << std::endl;
          abort();
        }
      }
    }

    typedef std::vector<art::Handle<art::TriggerResults> > Trig;
    Trig prod;
    e.getManyByType(prod);

    if (expected_trigger_previous_.size() > 0) {

      if (prod.size() == 0) {
        std::cerr << "TestTriggerNames: "
             << "No TriggerResults object found, expected previous trigger results"
             << std::endl;
        abort();
      }

      // The TriggerResults object for this event has not been written into
      // the event yet. prod[0] is the TriggerResults for the most recent
      // previous process.  This assumes this is not run in an end path.

      Strings triggernames;
      art::ServiceHandle<art::TriggerNamesService> tns;
      if (tns->getTrigPaths(*prod[0], triggernames)) {
        if (triggernames.size() != expected_trigger_previous_.size()) {
          std::cerr << "TestTriggerNames: "
               << "Expected and actual previous trigger path lists not the same size" << std::endl;
          abort();
        }
        for (Strings::size_type i = 0; i < expected_trigger_previous_.size(); ++i) {
          if (triggernames[i] != expected_trigger_previous_[i]) {
            std::cerr << "TestTriggerNames: "
                 << "Expected and actual previous trigger paths don't match" << std::endl;
            abort();
          }
        }
      }
      else {
        std::cerr << "TestTriggerNames: "
             << "Failed finding trigger names from a previous process" << std::endl;
        abort();
      }

      // The provenance of the TriggerResults object should also contain the
      // parameter set ID for the parameter set that lists the trigger paths.
      // Test this by getting this parameter set and verifying the trigger
      // paths are the correct size.
      if (!streamerSource_) {
        ParameterSetID trigpathsID = prod[0].provenance()->product().psetID();
        pset::Registry* psetRegistry = pset::Registry::instance();
        ParameterSet trigpset;
        bool status = psetRegistry->getMapped(trigpathsID, trigpset);
        if (status) {
          Strings trigpaths = trigpset.get<Strings>("trigger_paths");
          if (trigpaths.size() != expected_trigger_previous_.size()) {
            std::cerr << "TestTriggerNames: Using provenance\n"
                 << "Expected and actual previous trigger path not the same size" << std::endl;
            abort();
          }
        }
        else {
          std::cerr << "TestTriggerNames: "
               << "Could not find trigger_paths parameter set in registry" << std::endl;
          abort();
        }
      }

      // Look again using the TriggerNames class instead
      // of going directly to the service.  Try it both
      // both ways, using the constructor and also using
      // the init function of a member object.

      TriggerNames triggerNames(*prod[0]);

      // In the tests this is intended for, the first
      // return should be true and all the subsequent
      // returns false, because the triggernames are the
      // same in all the events, so they only get updated
      // once if the init function is used.
      if (triggerNamesI_.init(*prod[0]) != status_) {
        std::cerr << "TestTriggerNames: "
             << "init returning incorrect value" << std::endl;
        abort();
      }
      status_ = false;

      Strings triggernames2 = triggerNames.triggerNames();
      if (triggernames2.size() != expected_trigger_previous_.size()) {
        std::cerr << "TestTriggerNames: While exercising TriggerNames class\n"
             << "Expected and actual previous trigger path lists not the same size" << std::endl;
        abort();
      }
      for (Strings::size_type i = 0; i < expected_trigger_previous_.size(); ++i) {
        if (triggernames2[i] != expected_trigger_previous_[i]) {
          std::cerr << "TestTriggerNames: While exercising TriggerNames class\n"
               << "Expected and actual previous trigger paths don't match" << std::endl;
          abort();
        }
        if (triggerNames.triggerName(i) != expected_trigger_previous_[i]) {
          std::cerr << "TestTriggerNames: While exercising TriggerNames class\n"
               << "name from index accessor\n"
               << "Expected and actual previous trigger paths don't match" << std::endl;
          abort();
        }
        // Exercise the object initialized with the init function and
        // at the same time the function that returns an index
        if ( i != triggerNamesI_.triggerIndex(expected_trigger_previous_[i])) {
          std::cerr << "TestTriggerNames: While exercising TriggerNames class\n"
               << "index from name accessor\n"
               << "Expected and actual previous trigger paths don't match" << std::endl;
          abort();
        }
        if (triggerNamesI_.size() != expected_trigger_previous_.size()) {
          std::cerr << "TestTriggerNames: While exercising TriggerNames class\n"
               << "Checking size accessor\n"
               << "Expected and actual previous trigger paths don't match" << std::endl;
          abort();
        }
      }
    }
  }

  void TestTriggerNames::endJob()
  {
  }
}

using arttest::TestTriggerNames;

DEFINE_ART_MODULE(TestTriggerNames)
