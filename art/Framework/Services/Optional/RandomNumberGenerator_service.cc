#include "art/Framework/Services/Optional/RandomNumberGenerator.h"
// vim: set sw=2 expandtab :

// ======================================================================
// Maintain multiple independent random number engines, including
// saving and restoring state.
// ======================================================================

#include "CLHEP/Random/DRand48Engine.h"
#include "CLHEP/Random/DualRand.h"
#include "CLHEP/Random/Hurd160Engine.h"
#include "CLHEP/Random/Hurd288Engine.h"
#include "CLHEP/Random/JamesRandom.h"
#include "CLHEP/Random/MTwistEngine.h"
#include "CLHEP/Random/MixMaxRng.h"
#include "CLHEP/Random/NonRandomEngine.h"
#include "CLHEP/Random/Random.h"
#include "CLHEP/Random/RanecuEngine.h"
#include "CLHEP/Random/Ranlux64Engine.h"
#include "CLHEP/Random/RanluxEngine.h"
#include "CLHEP/Random/RanshiEngine.h"
#include "CLHEP/Random/TripleRand.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Services/Registry/ActivityRegistry.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Persistency/Provenance/ModuleDescription.h"
#include "art/Utilities/Globals.h"
#include "art/Utilities/ScheduleID.h"
#include "art/Utilities/ScheduleIteration.h"
#include "cetlib/assert_only_one_thread.h"
#include "cetlib/container_algorithms.h"
#include "cetlib/no_delete.h"
#include "cetlib_except/exception.h"
#include "hep_concurrency/RecursiveMutex.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <fstream>
#include <string>
#include <vector>

using namespace std;
using namespace string_literals;
using namespace hep::concurrency;
using fhicl::ParameterSet;

namespace art {

  long constexpr RandomNumberGenerator::maxCLHEPSeed; // if not MixMaxRng
  long constexpr RandomNumberGenerator::useDefaultSeed;

  namespace {

    string
    qualify_engine_label(ScheduleID const sid,
                         string const& module_label,
                         string const& engine_label)
    {
      // Format is ModuleLabel:scheduleID:EngineLabel
      string label;
      label += module_label;
      label += ':';
      label += std::to_string(sid.id());
      label += ':';
      label += engine_label;
      return label;
    }

    template <class DesiredEngineType>
    shared_ptr<CLHEP::HepRandomEngine>
    manufacture_an_engine(long const seed)
    {
      shared_ptr<CLHEP::HepRandomEngine> ret;
      if (seed == RandomNumberGenerator::useDefaultSeed) {
        ret.reset(new DesiredEngineType);
      } else {
        ret.reset(new DesiredEngineType(seed));
      }
      return ret;
    }

    shared_ptr<CLHEP::HepRandomEngine>
    engine_factory(string const& kind_of_engine_to_make, long const seed)
    {
#define MANUFACTURE(ENGINE)                                                    \
  if (kind_of_engine_to_make == string{#ENGINE}) {                             \
    return manufacture_an_engine<CLHEP::ENGINE>(seed);                         \
  }
      MANUFACTURE(DRand48Engine)
      MANUFACTURE(DualRand)
      MANUFACTURE(Hurd160Engine)
      MANUFACTURE(Hurd288Engine)
      MANUFACTURE(HepJamesRandom)
      MANUFACTURE(MixMaxRng)
      MANUFACTURE(MTwistEngine)
      MANUFACTURE(RanecuEngine)
      MANUFACTURE(Ranlux64Engine)
      MANUFACTURE(RanluxEngine)
      MANUFACTURE(RanshiEngine)
      MANUFACTURE(TripleRand)
#undef MANUFACTURE
      throw cet::exception("RANDOM")
        << "engine_factory():\n"
        << "Attempt to create engine of unknown kind \""
        << kind_of_engine_to_make << "\".\n";
    }

  } // unnamed namespace

  bool
  RandomNumberGenerator::invariant_holds_(ScheduleID const sid)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    return (data_[sid].dict_.size() == data_[sid].tracker_.size()) &&
           (data_[sid].dict_.size() == data_[sid].kind_.size());
  }

  RandomNumberGenerator::RandomNumberGenerator(Parameters const& config,
                                               ActivityRegistry& actReg)
    : defaultEngineKind_{config().defaultEngineKind()}
    , restoreStateLabel_{config().restoreStateLabel()}
    , saveToFilename_{config().saveTo()}
    , restoreFromFilename_{config().restoreFrom()}
    , debug_{config().debug()}
    , nPrint_{config().nPrint()}
  {
    actReg.sPostBeginJob.watch(this, &RandomNumberGenerator::postBeginJob);
    actReg.sPostEndJob.watch(this, &RandomNumberGenerator::postEndJob);
    actReg.sPreProcessEvent.watch(this,
                                  &RandomNumberGenerator::preProcessEvent);
    data_.resize(Globals::instance()->nschedules());
  }

  CLHEP::HepRandomEngine&
  RandomNumberGenerator::createEngine(ScheduleID const sid,
                                      std::string const& module_label,
                                      long const seed,
                                      string const& requested_engine_kind,
                                      string const& engine_label)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    if (!engine_creation_is_okay_) {
      throw cet::exception("RANDOM")
        << "RNGservice::createEngine():\n"
        << "Attempt to create engine \"" << engine_label << "\" is too late.\n";
    }
    if (sid.id() >= data_.size()) {
      throw cet::exception("RANDOM")
        << "RNGservice::createEngine():\n"
        << "Attempt to create engine with out-of-range ScheduleID: " << sid
        << "\n";
    }
    string const& label = qualify_engine_label(sid, module_label, engine_label);
    if (data_[sid].tracker_.find(label) != data_[sid].tracker_.cend()) {
      throw cet::exception("RANDOM")
        << "RNGservice::createEngine():\n"
        << "Engine \"" << label << "\" has already been created.\n";
    }
    string engineKind{requested_engine_kind};
    if (requested_engine_kind.empty()) {
      engineKind = defaultEngineKind_;
    }

    validate_(engineKind, seed);

    shared_ptr<CLHEP::HepRandomEngine> eptr;
    if (engineKind == "G4Engine"s) {
      eptr = engine_factory(defaultEngineKind_, seed);
      // We set CLHEP's random-number engine to be of type
      // defaultEngineKind_.
      CLHEP::HepRandom::setTheEngine(eptr.get());
      if (seed != RandomNumberGenerator::useDefaultSeed) {
        CLHEP::HepRandom::setTheSeed(seed);
      }
    } else if (engineKind == "NonRandomEngine"s) {
      eptr = std::make_shared<CLHEP::NonRandomEngine>();
    } else {
      eptr = engine_factory(engineKind, seed);
    }
    if (!eptr) {
      throw cet::exception("RANDOM")
        << "RNGservice::createEngine():\n"
        << "Engine \"" << label << "\" could not be created.\n";
    }
    data_[sid].dict_[label] = eptr;
    data_[sid].tracker_[label] = EngineSource::Seed;
    data_[sid].kind_[label] = engineKind;
    mf::LogInfo{"RANDOM"} << "Instantiated " << engineKind << " engine \""
                          << label << "\" with "
                          << ((seed == useDefaultSeed) ? "default seed " :
                                                         "seed ")
                          << seed << '.';
    assert(invariant_holds_(sid) &&
           "RNGservice::createEngine() invariant failed");
    return *eptr;
  }

  void
  RandomNumberGenerator::validate_(
    std::string const& user_specified_engine_kind,
    long const user_specified_seed) noexcept(false)
  {
    // The only time a user-specified seed can be negative is for
    // indicating to this service that the default seed for the
    // requested engine kind should be used.
    if (user_specified_seed == useDefaultSeed)
      return;

    if (user_specified_seed < 0) {
      throw cet::exception("RANGE") << "RNGservice::throw_if_invalid_seed():\n"
                                    << "Seed " << user_specified_seed
                                    << " is not permitted to be negative.\n";
    }

    if (user_specified_seed <= maxCLHEPSeed)
      return;

    // For now, only MixMaxRng engines can be constructed with a seed
    // value greater than maxCLHEPSeed.
    if (user_specified_engine_kind == "MixMaxRng"s)
      return;

    if (user_specified_engine_kind == "G4Engine"s &&
        defaultEngineKind_ == "MixMaxRng"s)
      return;

    throw cet::exception("RANGE")
      << "RNGservice::throw_if_invalid_seed():\n"
      << "Seed " << user_specified_seed << " exceeds permitted maximum of "
      << maxCLHEPSeed << " for engine type " << user_specified_engine_kind
      << ".\n";
  }

  void
  RandomNumberGenerator::print_() const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    static unsigned ncalls = 0;
    if (!debug_ || (++ncalls > nPrint_)) {
      return;
    }
    auto print_per_stream = [](size_t const i, auto const& d) {
      mf::LogInfo log{"RANDOM"};
      if (d.snapshot_.empty()) {
        log << "No snapshot has yet been made.\n";
        return;
      }
      log << "Snapshot information:";
      for (auto const& ss : d.snapshot_) {
        log << "\nEngine: " << ss.label() << "  Kind: " << ss.ekind()
            << "  Schedule ID: " << i << "  State size: " << ss.state().size();
      }
      log << "\n";
    };
    cet::for_all_with_index(data_, print_per_stream);
  }

  vector<RNGsnapshot> const&
  RandomNumberGenerator::accessSnapshot_(ScheduleID const sid) const
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    return data_[sid].snapshot_;
  }

  void
  RandomNumberGenerator::takeSnapshot_(ScheduleID const sid)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    mf::LogDebug log{"RANDOM"};
    log << "RNGservice::takeSnapshot_() of the following engine labels:\n";
    data_[sid].snapshot_.clear();
    for (auto const& pr : data_[sid].dict_) {
      string const& label = pr.first;
      shared_ptr<CLHEP::HepRandomEngine> const& eptr = pr.second;
      assert(eptr && "RNGservice::takeSnapshot_()");
      data_[sid].snapshot_.emplace_back(
        data_[sid].kind_[label], label, eptr->put());
      log << " | " << label;
    }
    log << " |\n";
  }

  void
  RandomNumberGenerator::restoreSnapshot_(ScheduleID const sid,
                                          Event const& event)
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    if (restoreStateLabel_.empty()) {
      return;
    }
    // access the saved-states product:
    auto const& saved =
      *event.getValidHandle<vector<RNGsnapshot>>(restoreStateLabel_);
    // restore engines from saved-states product:
    for (auto const& snapshot : saved) {
      string const& label = snapshot.label();
      mf::LogInfo log("RANDOM");
      log << "RNGservice::restoreSnapshot_(): label \"" << label << "\"";
      auto t = data_[sid].tracker_.find(label);
      if (t == data_[sid].tracker_.end()) {
        log << " could not be restored;\n"
            << "no established engine bears this label.\n";
        continue;
      }
      if (t->second == EngineSource::File) {
        throw cet::exception("RANDOM")
          << "RNGservice::restoreSnapshot_():\n"
          << "The state of engine \"" << label
          << "\" has been previously read from a file;\n"
          << "it is therefore not restorable from a snapshot product.\n";
      }
      shared_ptr<CLHEP::HepRandomEngine> ep{data_[sid].dict_[label]};
      assert(ep && "RNGservice::restoreSnapshot_()");
      data_[sid].tracker_[label] = EngineSource::Product;
      auto const& est = snapshot.restoreState();
      if (ep->get(est)) {
        log << " successfully restored.\n";
      } else {
        throw cet::exception("RANDOM")
          << "RNGservice::restoreSnapshot_():\n"
          << "Failed during restore of state of engine for \"" << label
          << "\"\n";
      }
    }
    assert(invariant_holds_(sid) && "RNGsnapshot::restoreSnapshot_()");
  }

  void
  RandomNumberGenerator::saveToFile_()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    if (saveToFilename_.empty()) {
      return;
    }
    CET_ASSERT_ONLY_ONE_THREAD();
    // access the file:
    ofstream outfile{saveToFilename_.c_str()};
    if (!outfile) {
      mf::LogWarning("RANDOM")
        << "Can't create/access file \"" << saveToFilename_ << "\"\n";
    }
    // save each engine:
    for (auto const& d : data_) {
      for (auto const& pr : d.dict_) {
        outfile << pr.first << '\n';
        auto const& eptr = pr.second;
        assert(eptr && "RNGservice::saveToFile_()");
        eptr->put(outfile);
        if (!outfile) {
          mf::LogWarning("RANDOM")
            << "This module's engine has not been saved;\n"
            << "file \"" << saveToFilename_ << "\" is likely now corrupted.\n";
        }
      }
    }
  }

  void
  RandomNumberGenerator::restoreFromFile_()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    if (restoreFromFilename_.empty()) {
      return;
    }
    CET_ASSERT_ONLY_ONE_THREAD();
    // access the file:
    ifstream infile{restoreFromFilename_.c_str()};
    if (!infile) {
      throw cet::exception("RANDOM")
        << "RNGservice::restoreFromFile_():\n"
        << "Can't open file \"" << restoreFromFilename_
        << "\" to initialize engines\n";
    }
    // restore engines:
    for (string label{}; infile >> label;) {
      // Get schedule ID from engine label
      assert(count(label.cbegin(), label.cend(), ':') == 2u);
      auto const p1 = label.find_first_of(':');
      auto const p2 = label.find_last_of(':');
      ScheduleID const sid{
        static_cast<ScheduleID::size_type>(stoi(label.substr(p1 + 1, p2)))};
      auto d = data_[sid].dict_.find(label);
      if (d == data_[sid].dict_.end()) {
        throw Exception(errors::Configuration, "RANDOM")
          << "Attempt to restore an engine with label " << label
          << " not configured in this job.\n";
      }
      assert((data_[sid].tracker_.find(label) != data_[sid].tracker_.cend()) &&
             "RNGservice::restoreFromFile_()");
      EngineSource& how{data_[sid].tracker_[label]};
      if (how == EngineSource::Seed) {
        auto& eptr = d->second;
        assert(eptr && "RNGservice::restoreFromFile_()");
        if (!eptr->get(infile)) {
          throw cet::exception("RANDOM")
            << "RNGservice::restoreFromFile_():\n"
            << "Failed during restore of state of engine for label " << label
            << "from file \"" << restoreFromFilename_ << "\"\n";
        }
        how = EngineSource::File;
      } else if (how == EngineSource::File) {
        throw Exception(errors::Configuration, "RANDOM")
          << "Engine state file contains two engine states with the same "
             "label: "
          << label << "\n.";
      } else {
        throw Exception(errors::LogicError, "RANDOM")
          << "Internal error: attempt to restore an engine state " << label
          << " from file\n"
          << "which was originally initialized via an unknown or impossible "
             "method.\n";
      }
      assert(invariant_holds_(sid) &&
             "RNGservice::restoreFromFile_() invariant failure");
    }
  }

  // Consider changing this to preBeginJob
  void
  RandomNumberGenerator::postBeginJob()
  {
    RecursiveMutexSentry sentry{mutex_, __func__};
    restoreFromFile_();
    engine_creation_is_okay_ = false;
  }

  void
  RandomNumberGenerator::preProcessEvent(Event const& e,
                                         ScheduleContext const sc)
  {
    auto const sid = sc.id();
    RecursiveMutexSentry sentry{mutex_, __func__};
    takeSnapshot_(sid);
    restoreSnapshot_(sid, e);
  }

  void
  RandomNumberGenerator::postEndJob()
  {
    // For normal termination, we wish to save the state at the *end* of
    // processing, not at the beginning of the last event.
    RecursiveMutexSentry sentry{mutex_, __func__};
    ScheduleIteration iteration(data_.size());
    iteration.for_each_schedule(
      [this](ScheduleID const sid) { takeSnapshot_(sid); });
    saveToFile_();
  }

} // namespace art

DEFINE_ART_SERVICE(art::RandomNumberGenerator)
