#ifndef art_Framework_Services_Optional_RandomNumberGenerator_h
#define art_Framework_Services_Optional_RandomNumberGenerator_h
// vim: set sw=2 expandtab :

// ==================================================================
// A service to maintain multiple independent random number engines.
// ==================================================================
//
// Via this RandomNumberGenerator, the CLHEP random number engines are
// made available such that a client module may establish and
// subsequently employ any number of independent engines, each of any
// of the CLHEP engine types.  Any created random number engines are
// owned either by art, or by an external party.  There is no use case
// of the RandomNumberGenerator service for which the user owns the
// engine.
//
// Any producer, analyzer, or filter module of the legacy or
// replicated threading types may use this service as needed (this
// header file is implicitly included for any such modules).  However,
// by design, source modules are permitted to make no use of this
// service.  In addition, no output modules, nor any modules of the
// shared threading type may use this service.
//
// Creating an engine
// ------------------
//
// Each engine to be used by a module must be created in that module's
// constructor.  Any modules that desire to create an engine must
// explicitly call the non-default base-class constructor (e.g.):
//
//   MyProducer(Parameters const& p,
//              ProcessingFrame const& frame)
//     : ReplicatedProducer{p, frame}
//   {}
//
// Creating an engine involves specifying:
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
// Each such call returns a non-const reference to the newly-created
// engine, that is owned by the framework.  The returned reference can
// be safely and immediately used by the CLHEP library to create a
// random number distribution.  For example:
//
//   CLHEP::RandFlat dist{createEngine(...)};
//
// In rare circumstances, the reference to the engine may be stored as
// a module-class data member.
//
// Creating the global engine
// --------------------------
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
// ----------------------------------
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
//   - For replicated modules, care must be taken *by the user* to
//     ensure that each module copy is initialized with the desired
//     seed.  If a module's configuration specifies a seed of 13597,
//     then a reasonable engine creation may look like:
//
//       createEngine(pset.get<int>("seed", 13597) +
//                      frame.scheduleID().id());
//
//     where the 'frame' is passed in as a replicated-module
//     constructor argument.  This way, each module copy for a
//     configured replicated module is guaranteed to have a different
//     seeds wrt. each other.
//
// Configuring the Service
// -----------------------
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
// ==================================================================

#include "CLHEP/Random/RandomEngine.h"
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
    using seed_t = long;

  private: // TYPES
  public:
    // Configuration
    struct Config {
      template <typename T>
      using Atom = fhicl::Atom<T>;
      using Name = fhicl::Name;
      using Comment = fhicl::Comment;
      Atom<std::string> restoreStateLabel{
        Name{"restoreStateLabel"},
        Comment{
          "The 'restoreStateLabel' parameter specifies the input tag used\n"
          "to restore the random number engine states, as stored in the\n"
          "data product produced by the RandomNumberSaver module.\n"},
        ""};
      Atom<std::string> saveTo{
        Name{"saveTo"},
        Comment{
          "The 'saveTo' and 'restoreFrom' parameters are filenames\n"
          "that indicate where the engine states should be saved-to or\n"
          "restored-from, respectively.  Engine states are saved at the\n"
          "end of an art job.  This allows a user to (e.g.) send a Ctrl+C\n"
          "signal during debugging, which will then save the engine states\n"
          "to the specified file during a graceful shutdown.  The user can\n"
          "then restore the engine states from the file and rerun the job,\n"
          "skipping to the appropriate event, but with the correct random\n"
          "engine states."},
        ""};
      Atom<std::string> restoreFrom{Name{"restoreFrom"}, ""};
      Atom<bool> debug{
        Name{"debug"},
        Comment{"Enable printout of random engine states for debugging."},
        false};
      Atom<unsigned> nPrint{
        Name{"nPrint"},
        Comment{
          "Limit the number of printouts to the specified value.\n"
          "This parameter can be specified only if 'debug' above is true."},
        [this] { return debug(); },
        10u};
    };

    using Parameters = ServiceTable<Config>;

    // Special Member Functions
    RandomNumberGenerator(Parameters const&, ActivityRegistry&);
    RandomNumberGenerator(RandomNumberGenerator const&) = delete;
    RandomNumberGenerator(RandomNumberGenerator&&) = delete;
    RandomNumberGenerator& operator=(RandomNumberGenerator const&) = delete;
    RandomNumberGenerator& operator=(RandomNumberGenerator&&) = delete;

  private:
    // Engine establishment
    // Accessible to user modules through friendship. Should only be used in
    // ctors.
    CLHEP::HepRandomEngine& createEngine(
      ScheduleID sid,
      std::string const& module_label,
      long seed,
      std::string const& kind_of_engine_to_make = defaultEngineKind,
      std::string const& engine_label = {});

    // Snapshot management helpers
    void takeSnapshot_(ScheduleID);
    void restoreSnapshot_(ScheduleID, Event const&);
    std::vector<RNGsnapshot> const& accessSnapshot_(ScheduleID) const;

    // File management helpers
    // TODO: Determine if this facility is necessary.
    void saveToFile_();
    void restoreFromFile_();

    // Debugging helpers
    void print_() const;
    bool invariant_holds_(ScheduleID);

    // Callbacks from the framework
    void preProcessEvent(Event const&, ScheduleContext);
    void postProcessEvent(Event const&, ScheduleContext);
    void postBeginJob();
    void postEndJob();

    // Protects all data members.
    mutable hep::concurrency::RecursiveMutex mutex_{"art::rng::mutex_"};

    // Product key for restoring from a snapshot
    std::string const restoreStateLabel_;

    // File name for saving state
    std::string const saveToFilename_;

    // File name for restoring state
    std::string const restoreFromFilename_;

    // Tracing and debug controls
    bool const debug_;
    unsigned const nPrint_;

    // Guard against tardy engine creation
    bool engine_creation_is_okay_{true};

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
    PerScheduleContainer<ScheduleData> data_;
  };

} // namespace art

DECLARE_ART_SERVICE(art::RandomNumberGenerator, LEGACY)

#endif /* art_Framework_Services_Optional_RandomNumberGenerator_h */

// Local Variables:
// mode: c++
// End:
