#include "art/Framework/Core/PrincipalCache.h"

#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Utilities/Exception.h"

namespace art {

  PrincipalCache::PrincipalCache() :
    runPrincipals_         ( ),
    subRunPrincipals_      ( ),
    currentRunPrincipal_   ( ),
    currentSubRunPrincipal_( )
  { }

  RunPrincipal & PrincipalCache::runPrincipal(RunID run) {
    auto iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  RunPrincipal const& PrincipalCache::runPrincipal(RunID run) const {
    auto iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  std::shared_ptr<RunPrincipal> PrincipalCache::runPrincipalPtr(RunID run) {
    auto iter = runPrincipals_.find(run);
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

  SubRunPrincipal & PrincipalCache::subRunPrincipal(SubRunID const & sr) {
    auto iter = subRunPrincipals_.find(sr);
    if (iter == subRunPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested a subRun that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  SubRunPrincipal const& PrincipalCache::subRunPrincipal(SubRunID const & sr) const {
    auto iter = subRunPrincipals_.find(sr);
    if (iter == subRunPrincipals_.end()) {
      throw art::Exception(art::errors::LogicError)
        << "PrincipalCache::subRunPrincipal\n"
        << "Requested a subRun that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  std::shared_ptr<SubRunPrincipal> PrincipalCache::subRunPrincipalPtr(SubRunID const & sr) {
    auto iter = subRunPrincipals_.find(sr);
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
    auto iter = runPrincipals_.find(rp->id());
    // For random access input, we require that the current run
    // principal be the last inserted regardless of whether it has been
    // seen before.
    currentRunPrincipal_ = rp;
    if (iter == runPrincipals_.end()) {
      runPrincipals_[rp->id()] = rp;
      return true;
    }

    iter->second->mergeRun(rp);
    currentRunPrincipal_ = iter->second;

    return true;
  }

  bool PrincipalCache::insert(std::shared_ptr<SubRunPrincipal> srp) {
    auto srid = srp->id();
    auto iter = subRunPrincipals_.find(srid);
    // For random access input, we require that the current run
    // principal be the last inserted regardless of whether it has been
    // seen before.
    currentSubRunPrincipal_ = srp;
    if (iter == subRunPrincipals_.end()) {
      subRunPrincipals_[srid] = srp;
      return true;
    }

    iter->second->mergeSubRun(srp);
    currentSubRunPrincipal_ = iter->second;

    return true;
  }

  bool PrincipalCache::noMoreRuns() const {
    return runPrincipals_.empty();
  }

  bool PrincipalCache::noMoreSubRuns() const {
    return subRunPrincipals_.empty();
  }

  RunPrincipal & PrincipalCache::lowestRun() const {
    auto iter = runPrincipals_.begin();
    return *iter->second.get();
  }

  SubRunPrincipal & PrincipalCache::lowestSubRun() const {
    auto iter = subRunPrincipals_.begin();
    return *iter->second.get();
  }

  void PrincipalCache::deleteRun(RunID run) {
    runPrincipals_.erase(runPrincipals_.find(run));
  }

  void PrincipalCache::deleteSubRun(SubRunID const & sr) {
    subRunPrincipals_.erase(subRunPrincipals_.find(sr));
  }

  void PrincipalCache::deleteAllPrincipals()
  {
    runPrincipals_.clear();
    subRunPrincipals_.clear();
  }
}
