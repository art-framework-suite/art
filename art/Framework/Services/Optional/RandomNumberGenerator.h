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
//   CLHEP::RandFlat dist{createEngine(...)};
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
//       createEngine(pset.get<int>("seed", 13597));
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
#include "art/Persistency/Provenance/ScheduleContext.h"
#include "art/Utilities/PerScheduleContainer.h"
#include "canvas/Persistency/Common/RNGsnapshot.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Name.h"
#include "hep_concurrency/RecursiveMutex.h"

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
  class RandomNumberSaver;

  namespace detail {
    class EngineCreator;
  }

  class RandomNumberGenerator {

    friend class EventProcessor;
    friend class detail::EngineCreator;
    friend class RandomNumberSaver;

  public: // TYPES
    // Used by createEngine, restoreSnapshot_, and restoreFromFile_.
    enum class EngineSource { Seed = 1, File = 2, Product = 3 };

  public: // CONSTANTS
    static std::string const defaultEngineKind /*  = "HepJamesRandom" */;
    static long constexpr maxCLHEPSeed{900000000};
    static long constexpr useDefaultSeed{-1};
    using base_engine_t = CLHEP::HepRandomEngine;

  private: // TYPES
    // Per-schedule data
    struct ScheduleData {
      // The labeled random number engines for this stream.
      // Indexed by engine label.
      std::map<std::string, std::shared_ptr<CLHEP::HepRandomEngine>> dict_{};

      // The most recent source of the labeled random number engines for this
      // stream. Indexed by engine label. When EngineSource == Seed, this means
      // an engine with the given label has been created by createEngine(sid,
      // seed, ...). When EngineSource == File, this means the engine was
      // created by restoring it from a file. When EngineSource == Product, this
      // means the engine was created by restoring it from a snapshot data
      // product with module label "restoreStateLabel".
      std::map<std::string, EngineSource> tracker_{};

      // The requested engine kind for each labeled random number engine for
      // this stream. Indexed by engine label.
      std::map<std::string, std::string> kind_{};

      // The random engine number state snapshots taken for this stream.
      std::vector<RNGsnapshot> snapshot_{};
    };

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

  public: // Special Member Functions
    RandomNumberGenerator(Parameters const&, ActivityRegistry&);
    RandomNumberGenerator(RandomNumberGenerator const&) = delete;
    RandomNumberGenerator(RandomNumberGenerator&&) = delete;
    RandomNumberGenerator& operator=(RandomNumberGenerator const&) = delete;
    RandomNumberGenerator& operator=(RandomNumberGenerator&&) = delete;

  public: // API -- Engine access
    // Note: Could remove if we do not need to support access from engines
    // restored from file.
    CLHEP::HepRandomEngine& getEngine(
      ScheduleID sid,
      std::string const& module_label,
      std::string const& engine_label = {}) const;

  private: // Engine establishment
    // Accessible to user modules through friendship. Should only be used in
    // ctors.
    CLHEP::HepRandomEngine& createEngine(
      ScheduleID sid,
      std::string const& module_label,
      long seed,
      std::string const& kind_of_engine_to_make = defaultEngineKind,
      std::string const& engine_label = {});

  private: // Snapshot management helpers
    void takeSnapshot_(ScheduleID);
    void restoreSnapshot_(ScheduleID, Event const&);
    std::vector<RNGsnapshot> const& accessSnapshot_(ScheduleID) const;

  private: // File management helpers
    // TODO: Determine if this facility is necessary.
    void saveToFile_();
    void restoreFromFile_();

  private: // Debugging helpers
    void print_() const;
    bool invariant_holds_(ScheduleID);

  private: // Callbacks from the framework
    void preProcessEvent(Event const&, ScheduleContext);
    void postProcessEvent(Event const&, ScheduleContext);
    void postBeginJob();
    void postEndJob();

  private:
    // Protects all data members.
    mutable hep::concurrency::RecursiveMutex mutex_{"art::rng::mutex_"};

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

    // Guard against tardy engine creation
    bool engine_creation_is_okay_{true};

    // Per-schedule data
    PerScheduleContainer<ScheduleData> data_;
  };

} // namespace art

DECLARE_ART_SERVICE(art::RandomNumberGenerator, LEGACY)

#endif /* art_Framework_Services_Optional_RandomNumberGenerator_h */

// Local Variables:
// mode: c++
// End:
