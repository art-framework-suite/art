#ifndef art_Framework_Services_Optional_RandomNumberGenerator_h
#define art_Framework_Services_Optional_RandomNumberGenerator_h
// vim: set sw=2 expandtab :

//
// A Service to maintain multiple independent random number engines.
//
// Introduction
//
// Via this RandomNumberGenerator, the CLHEP random number engines are
// made available such that a client module may establish and
// subsequently employ any number of independent engines, each of any
// of the CLHEP engine types.
//
// Any producer, analyzer, or filter module may freely use this
// Service as desired.  However, by design, source modules are
// permitted to make no use of this Service.
//
// Creating an engine
//
// MT FIXME: This will have to change because a module ctor currently
//           has no way of knowing what schedule (if any) it is being
//           created for.
//
// Each engine to be used by a module must be created in that module's
// constructor.  Creating an engine involves specifying:
//   - An integer seed value to initialize the engine's state
//   - The desired kind of engine ("HepJamesRandom" by default)
//   - A label string (empty by default)
// Within a module, no two engines may share an identical label.
//
// The above three items of information are supplied in a single call
// to a function named createEngine().  Because two of the three items
// have defaults (and may therefore be omitted), the call may take any
// of several forms; the following three examples therefore have
// equivalent effect:
//
//   createEngine(seed)
//   createEngine(seed, "HepJamesRandom")
//   createEngine(seed, "HepJamesRandom", "")
//
// As a convenience, each such call returns a reference to the newly-
// created engine; this is the same reference that would be returned
// from each subsequent corresponding getEngine() call (see below).
// Therefore, if it is convenient to do so, the reference returned
// from a call to createEngine() can be safely used right away as
// illustrated below.  If there is no immediate need for it, this
// returned reference can instead be safely ignored.
//
// Note that the createEngine() function is implicitly available to
// any producer, analyzer, or filter module; no additional header need
// be #included.
//
// Here is an example of a recommended practice in which the result of
// a createEngine() call is used right away (arguments elided for
// clarity):
//
//   CLHEP::RandFlat dist {createEngine(...)};
//
// Creating the global engine
//
// CLHEP provides the notion of a global engine, and Geant4 makes use
// of this feature.  It is recommended that the designated Geant4
// module (and no other) should create this global engine.
//
// The Service recognizes the notation "G4Engine" to create the engine
// that is used by Geant4.  It is strongly recommended to ignore the
// resulting engine reference, leaving exclusive future engine use to
// Geant4 itself:
//
//   createEngine(seed, "G4Engine");
//
// Digression: obtaining a seed value
//
// As noted above, creating an engine involves specifying a seed
// value.  Determining this value is at the discretion of the module
// creating the engine, and can be done in any of several manners.
// Here are some possibilities to get you started:
//
//   - Each CLHEP engine has a default seed value.  To specify the use
//     of this default seed, use the magic value -1 as the seed
//     argument in the createEngine() call:
//
//       createEngine(-1);
//
//   - Specify a (non-negative) constant of your choice as the seed
//     argument in the createEngine() call:
//
//       createEngine(13597);
//
//   - Obtain a seed value from the module's ParameterSet, typically
//     with some fallback value in case the ParameterSet omits the
//     specified parameter:
//
//       createEngine(pset.get<int>("seed",13597));
//
//   - Obtain a seed value from the module's ParameterSet via a helper
//     function, get_seed_value(), provided by the framework.  Since
//     this helper has defaults, each of the following calls has
//     equivalent effect:
//
//       createEngine(get_seed_value(pset));
//       createEngine(get_seed_value(pset,"seed"));
//       createEngine(get_seed_value(pset,"seed",-1));
//
// Service handles
//
// To gain general access to this RandomNumberGenerator, a module uses
// the facilities of the Service subsystem.  Thus, after #including
// the RandomNumberGenerator header, a variable definition such as the
// following will obtain a handle to this Service:
//
//   art::ServiceHandle<art::RandomNumberGenerator> rng;
//
// Thereafter, most functionality of this Service is available via
// this variable.  All handles to this Service are equivalent; a
// client may define as many or as few handles as desired.
//
// A module that has no need for this Service need not obtain any such
// handle at all.  Similarly, a module that creates an engine and
// makes use of the reference returned by the createEngine() call will
// also likely need obtain no handle.
//
// Accessing an engine
//
// To obtain access to a previously-established engine (see above),
// call the Service's getEngine function.  The call takes a single
// argument, namely the label that had been used when the engine was
// established.  If omitted, an empty label is used by default:
//
//   auto& engine = rng->getEngine();
//
// Note that the Service automatically knows which module is making
// the access request, and will return a reference to the proper
// engine for the current module, using the label to disambiguate if
// the module has established more than one engine.
//
// Configuring the Service
//
// TODO: draft this section
//
// TODO: assess the following remarks from the placeholder
// implementation and determine whether they are still valid/useful.
//
// When a separate Producer module is also included in the path, the
// state of all the engines managed by this service can be saved to
// the event.  Then in a later process, the RandomNumberGenerator is
// capable of restoring the state of the engines from the event in
// order to be able to exactly reproduce the earlier process.
//

#include "art/Framework/Services/Registry/ServiceMacros.h"
#include "art/Framework/Services/Registry/ServiceTable.h"
#include "canvas/Persistency/Common/RNGsnapshot.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace CLHEP {

  class HepRandomEngine;

} // namespace CLHEP

namespace art {

  class ActivityRegistry;
  class Event;
  class EventID;
  class Timestamp;

  class EventProcessor;
  class ModuleBase;
  class RandomNumberSaver;

  namespace test {

    class ConcurrentEngineRetrieval;

  } // namespace test

  class RandomNumberGenerator {

    friend class EventProcessor;
    friend class ModuleBase;
    friend class RandomNumberSaver;
    friend class test::ConcurrentEngineRetrieval;

  public: // TYPES
    // Used by createEngine.
    // Used by restoreSnapshot_.
    // Used by restoreFromFile_.
    enum class EngineSource { Seed = 1, File, Product };

  public: // CONFIGURATION
    struct Config {
      fhicl::Atom<std::string> restoreStateLabel{
        fhicl::Name{"restoreStateLabel"},
        ""};
      fhicl::Atom<std::string> saveTo{fhicl::Name{"saveTo"}, ""};
      fhicl::Atom<std::string> restoreFrom{fhicl::Name{"restoreFrom"}, ""};
      fhicl::Atom<unsigned> nPrint{fhicl::Name{"nPrint"}, 10u};
      fhicl::Atom<bool> debug{fhicl::Name{"debug"}, false};
    };

    using Parameters = ServiceTable<Config>;

  public: // MEMBER FUNCTIONS -- Special Member Functions
    RandomNumberGenerator(Parameters const&, ActivityRegistry&);

    RandomNumberGenerator(RandomNumberGenerator const&) = delete;

    RandomNumberGenerator(RandomNumberGenerator&&) = delete;

    RandomNumberGenerator& operator=(RandomNumberGenerator const&) = delete;

    RandomNumberGenerator& operator=(RandomNumberGenerator&&) = delete;

  public: // MEMBER FUNCTIONS -- Engine access
    // TODO: Could remove if we do not need to support access from
    //       engines restored from file.

    CLHEP::HepRandomEngine& getEngine() const;

    CLHEP::HepRandomEngine& getEngine(std::string const& engine_label) const;

  private: // TYPES
    // Per-schedule data
    struct ScheduleData {

      // Indexed by engine label.
      std::map<std::string, std::shared_ptr<CLHEP::HepRandomEngine>> dict_{};

      // Indexed by engine label.
      // When EngineSource == Seed, this means an engine with the
      // given label has been created by createEngine(si, seed, ...).
      // When EngineSource == File, this means the engine was
      // created by restoring it from a file.
      // When EngineSource == Product, this means the engine was
      // created by restoring it from a snapshot data product
      // with module label "restoreStateLabel".
      std::map<std::string, EngineSource> tracker_{};

      // Indexed by engine label.
      std::map<std::string, std::string> kind_{};

      std::vector<RNGsnapshot> snapshot_{};
    };

  private: // MEMBER FUNCTIONS -- Engine establishment
    CLHEP::HepRandomEngine& createEngine(ScheduleID scheduleID, long seed);

    CLHEP::HepRandomEngine& createEngine(
      ScheduleID scheduleID,
      long seed,
      std::string const& kind_of_engine_to_make);

    CLHEP::HepRandomEngine& createEngine(ScheduleID scheduleID,
                                         long seed,
                                         std::string kind_of_engine_to_make,
                                         std::string const& engine_label);

    CLHEP::HepRandomEngine& getEngine(
      ScheduleID scheduleID,
      std::string const& engine_label = {}) const;

    // --- MT-TODO: Only for testing
    //     For testing multi-schedule parallization of this service, the
    //     requested number of schedules is not expanded UNLESS the
    //     expandToNSchedules() function is called by a friend.
    void
    expandToNSchedules(unsigned const n)
    {
      data_.resize(n);
    }

  private: // MEMBER FUNCTIONS -- Snapshot management helpers
    void takeSnapshot_(ScheduleID scheduleID);

    void restoreSnapshot_(ScheduleID scheduleID, Event const&);

    std::vector<RNGsnapshot> const&
    accessSnapshot_(ScheduleID const scheduleID) const
    {
      return data_[scheduleID.id()].snapshot_;
    }

  private: // MEMBER FUNCTIONS -- File management helpers
    // TODO: Determine if this facility is necessary.

    void saveToFile_();

    void restoreFromFile_();

  private: // MEMBER FUNCTIONS -- Debugging helpers
    void print_() const;

    bool invariant_holds_(ScheduleID scheduleID);

  private: // MEMBER FUNCTIONS -- Callbacks
    void preProcessEvent(Event const&);

    void postProcessEvent(Event const&);

    void postBeginJob();

    void postEndJob();

  private: // MEMBER DATA -- Guard against tardy engine creation
    bool engine_creation_is_okay_{true};

  private:
    std::vector<ScheduleData> data_;

    // Product key for restoring from a snapshot
    std::string const restoreStateLabel_;

    // File name for saving state
    std::string const saveToFilename_;

    // File name for restoring state
    std::string const restoreFromFilename_;

    // Tracing and debug controls
    unsigned const nPrint_;

    // Tracing and debug controls
    bool const debug_;
  };

} // namespace art

DECLARE_ART_SERVICE(art::RandomNumberGenerator, LEGACY)

#endif /* art_Framework_Services_Optional_RandomNumberGenerator_h */

// Local Variables:
// mode: c++
// End:
