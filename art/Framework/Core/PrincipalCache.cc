#include "art/Framework/Core/PrincipalCache.h"

#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/Exception.h"

namespace art {

  PrincipalCache::PrincipalCache() :
    runPrincipals_         ( ),
    subRunPrincipals_      ( ),
    currentRunPrincipal_   ( ),
    currentSubRunPrincipal_( )
  { }

  RunPrincipal & PrincipalCache::runPrincipal(RunNumber_t run) {
    RunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  RunPrincipal const& PrincipalCache::runPrincipal(RunNumber_t run) const {
    ConstRunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  std::shared_ptr<RunPrincipal> PrincipalCache::runPrincipalPtr(RunNumber_t run) {
    RunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipalPtr\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return iter->second;
  }

  RunPrincipal & PrincipalCache::runPrincipal() {
    if (currentRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentRunPrincipal_.get();
  }

  RunPrincipal const& PrincipalCache::runPrincipal() const {
    if (currentRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentRunPrincipal_.get();
  }

  std::shared_ptr<RunPrincipal> PrincipalCache::runPrincipalPtr() {
    if (currentRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipalPtr\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return currentRunPrincipal_;
  }

  SubRunPrincipal & PrincipalCache::subRunPrincipal(RunNumber_t run, SubRunNumber_t subRun) {
    SubRunKey key(run, subRun);
    SubRunIterator iter = subRunPrincipals_.find(key);
    if (iter == subRunPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested a subRun that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  SubRunPrincipal const& PrincipalCache::subRunPrincipal(RunNumber_t run, SubRunNumber_t subRun) const {
    SubRunKey key(run, subRun);
    ConstSubRunIterator iter = subRunPrincipals_.find(key);
    if (iter == subRunPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested a subRun that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  std::shared_ptr<SubRunPrincipal> PrincipalCache::subRunPrincipalPtr(RunNumber_t run, SubRunNumber_t subRun) {
    SubRunKey key(run, subRun);
    SubRunIterator iter = subRunPrincipals_.find(key);
    if (iter == subRunPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipalPtr\n"
        << "Requested a subRun that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return iter->second;
  }

  SubRunPrincipal & PrincipalCache::subRunPrincipal() {
    if (currentSubRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested current subRun and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentSubRunPrincipal_.get();
  }

  SubRunPrincipal const& PrincipalCache::subRunPrincipal() const {
    if (currentSubRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested current subRun and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentSubRunPrincipal_.get();
  }

  std::shared_ptr<SubRunPrincipal> PrincipalCache::subRunPrincipalPtr() {
    if (currentSubRunPrincipal_.get() == 0) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipalPtr\n"
        << "Requested current subRun and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return currentSubRunPrincipal_;
  }

  bool PrincipalCache::insert(std::shared_ptr<RunPrincipal> rp) {
    RunNumber_t run = rp->run();
    RunIterator iter = runPrincipals_.find(run);
    // For random access input, we require that the current run
    // principal be the last inserted regardless of whether it has been
    // seen before.
    currentRunPrincipal_ = rp;
    if (iter == runPrincipals_.end()) {
      runPrincipals_[run] = rp;
      return true;
    }

    iter->second->mergeRun(rp);
    currentRunPrincipal_ = iter->second;

    return true;
  }

  bool PrincipalCache::insert(std::shared_ptr<SubRunPrincipal> srp) {
    RunNumber_t run = srp->run();
    SubRunNumber_t subRun = srp->subRun();
    SubRunKey key(run, subRun);
    SubRunIterator iter = subRunPrincipals_.find(key);
    // For random access input, we require that the current run
    // principal be the last inserted regardless of whether it has been
    // seen before.
    currentSubRunPrincipal_ = srp;
    if (iter == subRunPrincipals_.end()) {
      subRunPrincipals_[key] = srp;
      return true;
    }

    iter->second->mergeSubRun(srp);
    currentSubRunPrincipal_ = iter->second;

    return true;
  }

  bool PrincipalCache::noMoreRuns() {
    return runPrincipals_.empty();
  }

  bool PrincipalCache::noMoreSubRuns() {
    return subRunPrincipals_.empty();
  }

  RunPrincipal const& PrincipalCache::lowestRun() const {
    ConstRunIterator iter = runPrincipals_.begin();
    return *iter->second.get();
  }

  SubRunPrincipal const& PrincipalCache::lowestSubRun() const {
    ConstSubRunIterator iter = subRunPrincipals_.begin();
    return *iter->second.get();
  }

  void PrincipalCache::deleteLowestRun() {
    runPrincipals_.erase(runPrincipals_.begin());
  }

  void PrincipalCache::deleteLowestSubRun() {
    subRunPrincipals_.erase(subRunPrincipals_.begin());
  }

  void PrincipalCache::deleteRun(RunNumber_t run) {
    runPrincipals_.erase(runPrincipals_.find(run));
  }

  void PrincipalCache::deleteSubRun(RunNumber_t run, SubRunNumber_t subRun) {
    subRunPrincipals_.erase(subRunPrincipals_.find(SubRunKey(run, subRun)));
  }
}
