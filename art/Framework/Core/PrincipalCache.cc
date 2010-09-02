

//

#include "art/Framework/Core/PrincipalCache.h"
#include "art/Framework/Core/SubRunPrincipal.h"
#include "art/Framework/Core/RunPrincipal.h"
#include "art/Utilities/EDMException.h"

namespace edm {

  PrincipalCache::PrincipalCache() { }

  PrincipalCache::~PrincipalCache() { }

  RunPrincipal & PrincipalCache::runPrincipal(int run) {
    RunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  RunPrincipal const& PrincipalCache::runPrincipal(int run) const {
    ConstRunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  boost::shared_ptr<RunPrincipal> PrincipalCache::runPrincipalPtr(int run) {
    RunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::runPrincipalPtr\n"
        << "Requested a run that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return iter->second;
  }

  RunPrincipal & PrincipalCache::runPrincipal() {
    if (currentRunPrincipal_.get() == 0) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentRunPrincipal_.get();
  }

  RunPrincipal const& PrincipalCache::runPrincipal() const {
    if (currentRunPrincipal_.get() == 0) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::runPrincipal\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentRunPrincipal_.get();
  }

  boost::shared_ptr<RunPrincipal> PrincipalCache::runPrincipalPtr() {
    if (currentRunPrincipal_.get() == 0) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::runPrincipalPtr\n"
        << "Requested current run and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return currentRunPrincipal_;
  }

  SubRunPrincipal & PrincipalCache::lumiPrincipal(int run, int lumi) {
    SubRunKey key(run, lumi);
    SubRunIterator iter = lumiPrincipals_.find(key);
    if (iter == lumiPrincipals_.end()) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::lumiPrincipal\n"
        << "Requested a lumi that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  SubRunPrincipal const& PrincipalCache::lumiPrincipal(int run, int lumi) const {
    SubRunKey key(run, lumi);
    ConstSubRunIterator iter = lumiPrincipals_.find(key);
    if (iter == lumiPrincipals_.end()) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::lumiPrincipal\n"
        << "Requested a lumi that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *iter->second.get();
  }

  boost::shared_ptr<SubRunPrincipal> PrincipalCache::lumiPrincipalPtr(int run, int lumi) {
    SubRunKey key(run, lumi);
    SubRunIterator iter = lumiPrincipals_.find(key);
    if (iter == lumiPrincipals_.end()) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::lumiPrincipalPtr\n"
        << "Requested a lumi that is not in the cache (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return iter->second;
  }

  SubRunPrincipal & PrincipalCache::lumiPrincipal() {
    if (currentLumiPrincipal_.get() == 0) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::lumiPrincipal\n"
        << "Requested current lumi and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentLumiPrincipal_.get();
  }

  SubRunPrincipal const& PrincipalCache::lumiPrincipal() const {
    if (currentLumiPrincipal_.get() == 0) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::lumiPrincipal\n"
        << "Requested current lumi and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return *currentLumiPrincipal_.get();
  }

  boost::shared_ptr<SubRunPrincipal> PrincipalCache::lumiPrincipalPtr() {
    if (currentLumiPrincipal_.get() == 0) {
      throw edm::Exception(edm::errors::LogicError)
        << "PrincipalCache::lumiPrincipalPtr\n"
        << "Requested current lumi and it is not initialized (should never happen)\n"
        << "Contact a Framework Developer\n";
    }
    return currentLumiPrincipal_;
  }

  bool PrincipalCache::insert(boost::shared_ptr<RunPrincipal> rp) {
    int run = rp->run();
    RunIterator iter = runPrincipals_.find(run);
    if (iter == runPrincipals_.end()) {
      runPrincipals_[run] = rp;
      currentRunPrincipal_ = rp;
      return true;
    }

    iter->second->mergeRun(rp);
    currentRunPrincipal_ = iter->second;

    return true;
  }

  bool PrincipalCache::insert(boost::shared_ptr<SubRunPrincipal> lbp) {
    int run = lbp->run();
    int lumi = lbp->luminosityBlock();
    SubRunKey key(run, lumi);
    SubRunIterator iter = lumiPrincipals_.find(key);
    if (iter == lumiPrincipals_.end()) {
      lumiPrincipals_[key] = lbp;
      currentLumiPrincipal_ = lbp;
      return true;
    }

    iter->second->mergeLuminosityBlock(lbp);
    currentLumiPrincipal_ = iter->second;

    return true;
  }

  bool PrincipalCache::noMoreRuns() {
    return runPrincipals_.empty();
  }

  bool PrincipalCache::noMoreLumis() {
    return lumiPrincipals_.empty();
  }

  RunPrincipal const& PrincipalCache::lowestRun() const {
    ConstRunIterator iter = runPrincipals_.begin();
    return *iter->second.get();
  }

  SubRunPrincipal const& PrincipalCache::lowestLumi() const {
    ConstSubRunIterator iter = lumiPrincipals_.begin();
    return *iter->second.get();
  }

  void PrincipalCache::deleteLowestRun() {
    runPrincipals_.erase(runPrincipals_.begin());
  }

  void PrincipalCache::deleteLowestLumi() {
    lumiPrincipals_.erase(lumiPrincipals_.begin());
  }

  void PrincipalCache::deleteRun(int run) {
    runPrincipals_.erase(runPrincipals_.find(run));
  }

  void PrincipalCache::deleteLumi(int run, int lumi) {
    lumiPrincipals_.erase(lumiPrincipals_.find(SubRunKey(run, lumi)));
  }
}
