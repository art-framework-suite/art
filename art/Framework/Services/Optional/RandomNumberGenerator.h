#ifndef art_Framework_Services_Optional_RandomNumberGenerator_h
#define art_Framework_Services_Optional_RandomNumberGenerator_h

// ======================================================================
//
// A Service to maintain multiple independent random number engines.
//
// ======================================================================
// Introduction
// ------------
//
// Via this RandomNumberGenerator, the CLHEP random number engines
// are made available such that a client module may establish and
// subsequently employ any number of independent engines, each of any
// of the CLHEP engine types.
//
// Any producer, analyzer, or filter module may freely use this Service
// as desired.  However, by design, source modules are permitted to make
// no use of this Service.
//
// ======================================================================
// Creating an engine
// ------------------
//
// Each engine to be used by a module must be created in that module's
// constructor.  Creating an engine involves specifying:
//   - An integer seed value to initialize the engine's state
//   - The desired kind of engine ("HepJamesRandom" by default)
//   - A label string (empty by default)
// Within a module, no two engines may share the identical label.
//
// The above three items of information are supplied in a single call to
// a function named createEngine().  Because two of the three items have
// defaults (and may therefore be omitted), the call may take any of
// several forms; the following three examples therefore have equivalent
// effect:
//
//   createEngine( seed )
//   createEngine( seed, "HepJamesRandom" )
//   createEngine( seed, "HepJamesRandom", "" )
//
// As a convenience, each such call returns a reference to the newly-
// created engine; this is the same reference that would be returned from
// each subsequent corresponding getEngine() call (see below).
// Therefore, if it is convenient to do so, the reference returned from a
// call to createEngine() can be safely used right away as illustrated
// below.  If there is no immediate need for it, this returned reference
// can instead be safely ignored.
//
// Note that the createEngine() function is implicitly available to any
// producer, analyzer, or filter module; no additional header need be
// #included.
//
// Here is an example of a recommended practice in which the result of a
// createEngine() call is used right away (arguments elided for clarity):
//
//   CLHEP::RandFlat dist( createEngine(...) );
//
// ======================================================================
// Creating the global engine
// --------------------------
//
// CLHEP provides the notion of a global engine, and Geant4 makes use of
// this feature.  It is recommended that the designated Geant4 module
// (and no other) should create this global engine.
//
// The Service recognizes the notation "G4Engine" to create the engine
// that is used by Geant4.  It is strongly recommended to ignore the
// resulting engine reference, leaving exclusive future engine use to
// Geant4 itself:
//
//   createEngine( seed, "G4Engine" );
//
// ======================================================================
// Digression: obtaining a seed value
// ----------------------------------
//
// As noted above, creating an engine involves specifying a seed value.
// Determining this value is at the discretion of the module creating
// the engine, and can be done in any of several manners.  Here are some
// possibilities to get you started:
//
//   - Each CLHEP engine has a default seed value.  To specify the use
//     of this default seed, use the magic value -1 as the seed argument
//     in the createEngine() call:
//
//       createEngine( -1 );
//
//   - Specify a (non-negative) constant of your choice as the seed
//     argument in the createEngine() call:
//
//       createEngine( 13597 );
//
//   - Obtain a seed value from the module's ParameterSet, typically
//     with some fallback value in case the ParameterSet omits the
//     specified parameter:
//
//       createEngine( pset.get<int>("seed",13597) );
//
//   - Obtain a seed value from the module's ParameterSet via a helper
//     function, get_seed_value(), provided by the framework.  Since this
//     helper has defaults, each of the following calls has equivalent
//     effect:
//
//       createEngine( get_seed_value(pset) );
//       createEngine( get_seed_value(pset,"seed") );
//       createEngine( get_seed_value(pset,"seed",-1) );
//
// ======================================================================
// Service handles
// ---------------
//
// To gain general access to this RandomNumberGenerator, a module uses
// the facilities of the Service subsystem.  Thus, after #including the
// RandomNumberGenerator header, a variable definition such as the
// following will obtain a handle to this Service:
//
//   art::ServiceHandle<art::RandomNumberGenerator>  rng;
//
// Thereafter, most functionality of this Service is available via this
// variable.  All handles to this Service are equivalent; a client may
// define as many or as few handles as desired.
//
// A module that has no need for this Service need not obtain any such
// handle at all.  Similarly, a module that creates an engine and makes
// use of the reference returned by the createEngine() call will also
// likely need obtain no handle.
//
// ======================================================================
// Accessing an engine
// -------------------
//
// To obtain access to a previously-established engine (see above), call
// the Service's getEngine function.  The call takes a single argument,
// namely the label that had been used when the engine was established.
// If omitted, an empty label is used by default:
//
//   CLHEP::HepRandomEngine & engine = rng -> getEngine();
//
// Note that the Service automatically knows which module is making the
// access request, and will return a reference to the proper engine for
// the current module, using the label to disambiguate if the module has
// established more than one engine.
//
// ======================================================================
// Configuring the Service
// -----------------------
//
// TODO: draft this section
// ======================================================================
// TODO: assess the following remarks from the placeholder implementation
// and determine whether they are still valid/useful.
//
// When a separate Producer module is also included in the path, the state
// of all the engines managed by this service can be saved to the event.
// Then in a later process, the RandomNumberGenerator is capable of restoring
// the state of the engines from the event in order to be able to exactly
// reproduce the earlier process.
//
// ======================================================================

#include "art/Persistency/Common/RNGsnapshot.h"
#include "cpp0x/memory"
#include "fhiclcpp/ParameterSet.h"
#include <map>
#include <string>
#include <vector>

namespace art {
  class ActivityRegistry;
  class Event;
  class EventID;
  class Timestamp;

  class EngineCreator;      // to be granted friendship
  class RandomNumberSaver;  // to be granted friendship
  class InputSource;        // to be granted friendship TODO: remove this

  class RandomNumberGenerator;
}

namespace CLHEP {
  class HepRandomEngine;
}

// ======================================================================

class art::RandomNumberGenerator
{
  friend class EngineCreator;
  friend class RandomNumberSaver;
  friend class InputSource;  // TODO: remove this

  // --- Prevent copying:
  RandomNumberGenerator( RandomNumberGenerator const & );
  void  operator = ( RandomNumberGenerator const & );

public:
  // --- CLHEP engine characteristics:
  typedef  CLHEP::HepRandomEngine            base_engine_t;
  typedef  long                              seed_t;
  typedef  RNGsnapshot::engine_state_t       engine_state_t;

  // --- Internal state characteristics:
  enum     init_t  { VIA_SEED=1, VIA_FILE, VIA_PRODUCT };
  typedef  RNGsnapshot::label_t              label_t;
  typedef  std::shared_ptr<base_engine_t>    eptr_t;
  typedef  std::map<label_t,eptr_t>          dict_t;
  typedef  std::map<label_t,init_t>          tracker_t;
  typedef  std::map<label_t,std::string>     kind_t;
  typedef  std::vector<RNGsnapshot>          snapshot_t;

  // --- C'tor/d'tor:
  RandomNumberGenerator( fhicl::ParameterSet const &
                       , art::ActivityRegistry     &
                       );

  // use compiler-generated d'tor

  // --- Engine access:
  base_engine_t &  getEngine( ) const;
  base_engine_t &  getEngine( label_t const &  engine_label ) const;

private:
  // --- Engine establishment:
  base_engine_t &
    createEngine( seed_t  seed );
  base_engine_t &
    createEngine( seed_t               seed
                , std::string const &  kind_of_engine_to_make
                );
  base_engine_t &
    createEngine( seed_t               seed
                , std::string          kind_of_engine_to_make
                , label_t const &      engine_label
                );

  // --- Snapshot management helpers:
  void  takeSnapshot_( );
  void  restoreSnapshot_( art::Event const & event );
  snapshot_t const &  accessSnapshot_( )  { return snapshot_; }

  // --- File management helpers:
  void  saveToFile_     ( );
  void  restoreFromFile_( );

  // --- Debugging helpers:
  void  print_( );
  bool  invariant_holds_( );

  // --- Callbacks:
  void  preProcessEvent( art::Event const &);
  void  postProcessEvent( art::Event const &);
  void  postBeginJob( );
  void  postEndJob( );

  // --- Guard against tardy engine creation:
  bool  engine_creation_is_okay_;

  // --- Per-module-instance information:
  dict_t     dict_;
  tracker_t  tracker_;
  kind_t     kind_;

  // --- Snapshot information:
  snapshot_t  snapshot_;

  // --- Product key for restoring from a snapshot:
  label_t  restoreStateLabel_;

  // --- File names for saving/restoring state:
  std::string  saveToFilename_;
  std::string  restoreFromFilename_;

  // --- Tracing and debug controls:
  int  nPrint_;
  bool debug_;

};  // RandomNumberGenerator

// ======================================================================

#endif /* art_Framework_Services_Optional_RandomNumberGenerator_h */

// Local Variables:
// mode: c++
// End:
