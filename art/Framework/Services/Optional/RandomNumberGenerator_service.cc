// ======================================================================
//
// Maintain multiple independent random number engines, including
// saving and restoring state.
//
// ======================================================================
//
// Notes
// -----
//
// 0) These notes predate this version.
//    TODO: review/apply/update these notes as/when time permits.
//
// 1) The CMS code on which is this modelled is available at
//    http://cmslxr.fnal.gov/lxr/source/IOMC/RandomEngine/src/RandomNumberGenerator.cc
//
// 2) CLHEP specifies that state will be returned as vector<unsigned long>.
//    The size of a long is machine dependent. If unsigned long is an
//    8 byte variable, only the least significant 4 bytes are filled
//    and the most significant 4 bytes are zero.  We need to store the
//    state with a machine independent size, which we choose to be
//    uint32_t. This conversion really belongs in the
//    RandomEngineState class but we are at the moment constrained by
//    the framework's HepRandomGenerator interface.
//
// ======================================================================

#include "art/Framework/Services/Optional/RandomNumberGenerator.h"

#include "CLHEP/Random/DRand48Engine.h"
#include "CLHEP/Random/DualRand.h"
#include "CLHEP/Random/Hurd160Engine.h"
#include "CLHEP/Random/Hurd288Engine.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/MTwistEngine.h"
#include "CLHEP/Random/NonRandomEngine.h"
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RandomEngine.h"
#include "CLHEP/Random/RanecuEngine.h"
#include "CLHEP/Random/Ranlux64Engine.h"
#include "CLHEP/Random/RanluxEngine.h"
#include "CLHEP/Random/RanshiEngine.h"
#include "CLHEP/Random/TripleRand.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Services/System/CurrentModule.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Principal/Handle.h"
#include "canvas/Persistency/Provenance/ModuleDescription.h"
#include "cetlib/assert_only_one_thread.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/exception.h"
#include "cetlib/no_delete.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <atomic>
#include <cassert>
#include <fstream>
#include <string>
#include <vector>

using art::RNGsnapshot;
using art::RandomNumberGenerator;
using fhicl::ParameterSet;
using std::ifstream;
using std::ofstream;
using std::string;

// ======================================================================

namespace {

  using RNGservice = RandomNumberGenerator;
  using eptr_t = RNGservice::eptr_t;
  using label_t = RNGservice::label_t;
  using seed_t = RNGservice::seed_t;
  using base_engine_t = RNGservice::base_engine_t;

  string const DEFAULT_ENGINE_KIND {"HepJamesRandom"};
  seed_t constexpr MAXIMUM_CLHEP_SEED {900000000};
  seed_t constexpr USE_DEFAULT_SEED {-1};
  RNGsnapshot const EMPTY_SNAPSHOT;

  struct G4Engine {};

  void
  throw_if_invalid_seed(seed_t const seed)
  {
    if (seed == USE_DEFAULT_SEED)
      return;
    if (seed > MAXIMUM_CLHEP_SEED)
      throw cet::exception("RANGE")
        << "RNGservice::throw_if_invalid_seed():\n"
        "Seed " << seed << " exceeds permitted maximum "
        << MAXIMUM_CLHEP_SEED << ".\n";
    if (seed < 0)  // too small for CLHEP
      throw cet::exception("RANGE")
        << "RNGservice::throw_if_invalid_seed():\n"
        "Seed " << seed << " is not permitted to be negative.\n";
  }

  inline label_t
  qualify_engine_label(unsigned const schedule_id, label_t const& engine_label)
  {
    // ModuleLabel:ScheduleID:EngineLabel
    std::string label {art::ServiceHandle<art::CurrentModule const>{}->label()};
    label += ':';
    label += std::to_string(schedule_id);
    label += ':';
    label += engine_label;
    return label;
  }

  template <class DesiredEngineType>
  inline eptr_t
  manufacture_an_engine(seed_t const seed)
  {
    return eptr_t{seed == USE_DEFAULT_SEED ? new DesiredEngineType : new DesiredEngineType(seed)};
  }

  template <>
  inline eptr_t
  manufacture_an_engine<CLHEP::NonRandomEngine>(seed_t /* unused */)
  {
    // no engine c'tor takes a seed:
    return std::make_shared<CLHEP::NonRandomEngine>();
  }

  template<>
  inline eptr_t
  manufacture_an_engine<G4Engine>(seed_t const seed)
  {
    if (seed != USE_DEFAULT_SEED)
      CLHEP::HepRandom::setTheSeed(seed);

    return eptr_t{CLHEP::HepRandom::getTheEngine(), cet::no_delete{}};
  }

  eptr_t
  engine_factory(string const& kind_of_engine_to_make, seed_t const seed)
  {
#define MANUFACTURE_EXPLICIT(KIND,TYPE)                 \
    if (kind_of_engine_to_make == string{KIND})         \
      return manufacture_an_engine<TYPE>(seed);
    MANUFACTURE_EXPLICIT("G4Engine", G4Engine)
#define MANUFACTURE_IMPLICIT(ENGINE) MANUFACTURE_EXPLICIT(#ENGINE,CLHEP::ENGINE)
      MANUFACTURE_IMPLICIT(DRand48Engine)
      MANUFACTURE_IMPLICIT(DualRand)
      MANUFACTURE_IMPLICIT(Hurd160Engine)
      MANUFACTURE_IMPLICIT(Hurd288Engine)
      MANUFACTURE_IMPLICIT(HepJamesRandom)
      MANUFACTURE_IMPLICIT(MTwistEngine)
      MANUFACTURE_IMPLICIT(NonRandomEngine)
      MANUFACTURE_IMPLICIT(RanecuEngine)
      MANUFACTURE_IMPLICIT(Ranlux64Engine)
      MANUFACTURE_IMPLICIT(RanluxEngine)
      MANUFACTURE_IMPLICIT(RanshiEngine)
      MANUFACTURE_IMPLICIT(TripleRand)
#undef MANUFACTURE_IMPLICIT
#undef MANUFACTURE_EXPLICIT

      throw cet::exception("RANDOM")
      << "engine_factory():\n"
      "Attempt to create engine of unknown kind \""
      << kind_of_engine_to_make << "\".\n";
  }  // engine_factory()

  void
  expand_if_abbrev_kind(string& requested_engine_kind)
  {
    if (requested_engine_kind.empty() ||
        requested_engine_kind == "DefaultEngine" ||
        requested_engine_kind == "JamesRandom") {
      requested_engine_kind = DEFAULT_ENGINE_KIND;
    }
  }

}  // namespace

// ======================================================================

RNGservice::RandomNumberGenerator(Parameters const& config,
                                  art::ActivityRegistry& reg)
  : restoreStateLabel_{config().restoreStateLabel()}
  , saveToFilename_{config().saveTo()}
  , restoreFromFilename_{config().restoreFrom()}
  , nPrint_{config().nPrint()}
  , debug_{config().debug()}
{
  reg.sPostBeginJob.watch   (this, &RNGservice::postBeginJob);
  reg.sPostEndJob.watch     (this, &RNGservice::postEndJob);
  reg.sPreProcessEvent.watch(this, &RNGservice::preProcessEvent);

  // Placeholder until we can actually query number of schedules.
  unsigned const nSchedules {1u};
  data_.resize(nSchedules);
}

// ----------------------------------------------------------------------

base_engine_t&
RNGservice::getEngine() const
{
  return getEngine(label_t{});
}

base_engine_t&
RNGservice::getEngine(label_t const& engine_label) const
{
  // Place holder until we can use a system that provides the right context.
  unsigned constexpr schedule_id {0u};
  label_t const& label = qualify_engine_label(schedule_id, engine_label);

  auto d = data_[schedule_id].dict_.find(label);
  if (d == data_[schedule_id].dict_.end()) {
    throw cet::exception("RANDOM")
      << "RNGservice::getEngine():\n"
      "The requested engine \"" << label << "\" has not been established.\n";
  }
  assert(d->second && "RNGservice::getEngine()");

  return *d->second;
}

// ======================================================================

bool
RNGservice::invariant_holds_(unsigned const schedule_id)
{
  auto const& d = data_[schedule_id];
  return d.dict_.size() == d.tracker_.size() &&
    d.dict_.size() == d.kind_.size();
}

// ----------------------------------------------------------------------

base_engine_t&
RNGservice::createEngine(unsigned const schedule_id,
                         seed_t const seed)
{
  return createEngine(schedule_id, seed, DEFAULT_ENGINE_KIND);
}


base_engine_t&
RNGservice::createEngine(unsigned const schedule_id,
                         seed_t const seed,
                         string const& requested_engine_kind)
{
  return createEngine(schedule_id, seed, requested_engine_kind, label_t{});
}


base_engine_t&
RNGservice::createEngine(unsigned const schedule_id,
                         seed_t const seed,
                         string requested_engine_kind,
                         label_t const& engine_label)
{
  label_t const& label = qualify_engine_label(schedule_id, engine_label);

  if (!engine_creation_is_okay_) {
    throw cet::exception("RANDOM")
      << "RNGservice::createEngine():\n"
      "Attempt to create engine \"" << label << "\" is too late.\n";
  }

  auto& d = data_[schedule_id];
  if (d.tracker_.find(label) != d.tracker_.cend()) {
    throw cet::exception("RANDOM")
      << "RNGservice::createEngine():\n"
      "Engine \"" << label << "\" has already been created.\n";
  }

  throw_if_invalid_seed(seed);
  expand_if_abbrev_kind(requested_engine_kind);
  eptr_t eptr {engine_factory(requested_engine_kind, seed)};
  assert(eptr && "RNGservice::createEngine()");
  d.dict_   [label] = eptr;
  d.tracker_[label] = VIA_SEED;
  d.kind_   [label] = requested_engine_kind;

  mf::LogInfo log {"RANDOM"};
  log << "Instantiated " << requested_engine_kind
      << " engine \"" << label
      << "\" with ";
  if (seed == USE_DEFAULT_SEED) log << "default seed " << seed;
  else                          log << "seed " << seed;
  log << ".\n";

  assert(invariant_holds_(schedule_id) && "RNGservice::createEngine()");
  return *eptr;

}  // createEngine<>()

// ----------------------------------------------------------------------
// The 'print_()' does not receive a schedule ID as an argument since
// it is only intended for debugging purposes.  Because of this, it is
// possible that data races can occur in a multi-threaded context.
// Since the only user of 'print_' is the RandomNumberSaver module, we
// place the burden of synchronizing on that module, and not on this
// service, which could be expensive.
void
RNGservice::print_() const
{
  static unsigned ncalls {};

  if (!debug_ || ++ncalls > nPrint_)
    return;

  auto print_per_stream = [](std::size_t const i, auto const& d) {
    mf::LogInfo log {"RANDOM"};
    if (d.snapshot_.empty()) {
      log << "No snapshot has yet been made.\n";
      return;
    }

    log << "Snapshot information:";
    for (auto const& ss : d.snapshot_) {
      log << "\nEngine: " << ss.label()
          << "  Kind: " << ss.ekind()
          << "  Schedule ID: " << i
          << "  State size: " << ss.state().size();
    }
    log << "\n";
  };

  cet::for_all_with_index(data_, print_per_stream);
}

// ----------------------------------------------------------------------

void
RNGservice::takeSnapshot_(unsigned const schedule_id)
{
  mf::LogDebug log {"RANDOM"};
  log << "RNGservice::takeSnapshot_() of the following engine labels:\n";

  auto& d = data_[schedule_id];
  d.snapshot_.clear();
  for (auto const& pr : d.dict_) {
    label_t const& label = pr.first;
    eptr_t const& eptr = pr.second;
    assert(eptr && "RNGservice::takeSnapshot_()");

    d.snapshot_.emplace_back(d.kind_[label], label, eptr->put());
    log << " | " << label;
  }
  log << " |\n";
}

// ----------------------------------------------------------------------

void
RNGservice::restoreSnapshot_(unsigned const schedule_id, art::Event const& event)
{
  if (restoreStateLabel_.empty())
    return;

  // access the saved-states product:
  using saved_t = std::vector<RNGsnapshot>;
  auto const& saved = *event.getValidHandle<saved_t>(restoreStateLabel_);

  auto& d = data_[schedule_id];

  // restore engines from saved-states product:
  for (auto const& snapshot : saved) {
    label_t const& label = snapshot.label();
    mf::LogInfo log("RANDOM");
    log << "RNGservice::restoreSnapshot_(): label \"" << label << "\"";

    auto t = d.tracker_.find(label);
    if (t == d.tracker_.end()) {
      log << " could not be restored;\n"
        "no established engine bears this label.\n";
      continue;
    }

    if (t->second == VIA_FILE) {
      throw cet::exception("RANDOM")
        << "RNGservice::restoreSnapshot_():\n"
        "The state of engine \"" << label
        << "\" has been previously read from a file;\n"
        "it is therefore not restorable from a snapshot product.\n";
    }

    eptr_t ep {d.dict_[label]};
    assert(ep && "RNGservice::restoreSnapshot_()");

    d.tracker_[label] = VIA_PRODUCT;
    auto const& est = snapshot.restoreState();
    if (ep->get(est)) {
      log << " successfully restored.\n";
    }
    else {
      throw cet::exception("RANDOM")
        << "RNGservice::restoreSnapshot_():\n"
        "Failed during restore of state of engine for \"" << label << "\"\n";
    }
  }  // for

  assert(invariant_holds_(schedule_id) && "RNGsnapshot::restoreSnapshot_()");
}  // restoreSnapshot_()

// ----------------------------------------------------------------------
void
RNGservice::saveToFile_()
{
  if (saveToFilename_.empty())
    return;

  CET_ASSERT_ONLY_ONE_THREAD();

  // access the file:
  ofstream outfile {saveToFilename_.c_str()};
  if (!outfile)
    mf::LogWarning("RANDOM")
      << "Can't create/access file \"" << saveToFilename_ << "\"\n";

  // save each engine:
  for (auto const& d : data_) {
    for (auto const& pr : d.dict_) {
      outfile << pr.first << '\n';
      eptr_t const& eptr = pr.second;
      assert(eptr && "RNGservice::saveToFile_()");

      eptr->put(outfile);
      if (!outfile)
        mf::LogWarning("RANDOM")
          << "This module's engine has not been saved;\n"
          "file \"" << saveToFilename_ << "\" is likely now corrupted.\n";
    }
  }
}  // saveToFile_()

// ----------------------------------------------------------------------
void
RNGservice::restoreFromFile_()
{
  if (restoreFromFilename_.empty())
    return;

  CET_ASSERT_ONLY_ONE_THREAD();

  // access the file:
  ifstream infile {restoreFromFilename_.c_str()};
  if (!infile)
    throw cet::exception("RANDOM")
      << "RNGservice::restoreFromFile_():\n"
      "Can't open file \"" << restoreFromFilename_
      << "\" to initialize engines\n";

  // restore engines:
  for (label_t label{}; infile >> label;) {
    // Get schedule ID from engine label
    assert(std::count(label.cbegin(), label.cend(), ':') == 2u);
    auto const p1 = label.find_first_of(':');
    auto const p2 = label.find_last_of(':');
    unsigned const schedule_id = std::stoul(label.substr(p1+1,p2));
    auto& data = data_[schedule_id];

    auto d = data.dict_.find(label);
    if (d == data.dict_.end()) {
      throw Exception(errors::Configuration, "RANDOM")
        << "Attempt to restore an engine with label "
        << label
        << " not configured in this job.\n";
    }

    eptr_t& eptr = d->second;
    assert(eptr && "RNGservice::restoreFromFile_()" );
    assert(data.tracker_.find(label) != data.tracker_.cend() && "RNGservice::restoreFromFile_()");
    init_t& how {data.tracker_[label]};
    if (how == VIA_SEED) { // OK
      if (!eptr->get(infile)) {
        throw cet::exception("RANDOM")
          << "RNGservice::restoreFromFile_():\n"
          << "Failed during restore of state of engine for label "
          << label
          << "from file \"" << restoreFromFilename_ << "\"\n";
      }
      how = VIA_FILE;
    } else if (how == VIA_FILE) {
      throw Exception(errors::Configuration, "RANDOM")
        << "Engine state file contains two engine states with the "
        << "same label: "
        << label
        << "\n.";
    } else {
      throw Exception(errors::LogicError, "RANDOM")
        << "Internal error: attempt to restore an engine state "
        << label
        << " from file\nwhich was originally initialized via an "
        << " unknown or impossible method.\n";
    }
    assert(invariant_holds_(schedule_id) && "RNGservice::restoreFromFile_()");
  }
}  // restoreFromFile_()

// ----------------------------------------------------------------------

void
RNGservice::postBeginJob()
{
  restoreFromFile_();
  engine_creation_is_okay_ = false;
}

void
RNGservice::preProcessEvent(art::Event const& e)
{
  unsigned const schedule_id {0u};
  takeSnapshot_(schedule_id);
  restoreSnapshot_(schedule_id, e);
}

void
RNGservice::postEndJob()
{
  // For normal termination, we wish to save the state at the *end* of
  // processing, not at the beginning of the last event.
  unsigned const schedule_id {0u};
  takeSnapshot_(schedule_id);
  saveToFile_();
}

// ======================================================================
DEFINE_ART_SERVICE(RandomNumberGenerator)
