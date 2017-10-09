#include "art/Framework/Core/CachedProducts.h"

#include "cetlib/container_algorithms.h"

#include <map>
#include <string>
#include <vector>

using namespace cet;
using namespace std;

void
art::detail::CachedProducts::setupDefault(vector<string> const& triggernames)
{
  // Setup to accept everything.
  vector<string> paths;
  EventSelector ES(paths, triggernames);
  p_and_e_selectors_.emplace_back("", ES);
}

void
art::detail::CachedProducts::setup(
  vector<pair<string, string>> const& path_specs,
  vector<string> const& triggernames,
  string const& process_name)
{
  // Turn the passed path specs into a map of process name to
  // a vector of trigger names.
  map<string, vector<string>> paths_for_process;
  for (auto const& pr : path_specs) {
    auto const& pname = pr.first.empty() ? process_name : pr.first;
    paths_for_process[pname].push_back(pr.second);
  }

  // Now go through all the process names found, and create an event
  // selector for each one.
  for (auto const& pr : paths_for_process) {
    auto const& pname = pr.first;
    auto const& paths = pr.second;
    if (pname == process_name) {
      // For the passed process name we have been given the trigger names.
      p_and_e_selectors_.emplace_back(pname,
                                      EventSelector{paths, triggernames});
      continue;
    }
    // For other process names we do not know the trigger names.
    p_and_e_selectors_.emplace_back(pname, EventSelector{paths});
  }
}

bool
art::detail::CachedProducts::wantEvent(Event const& ev)
{
  // Get all the TriggerResults objects before we test any for a
  // match, because we have to deal with the possibility there may be
  // more than one.  Note that the existence of more than one object
  // in the event is intended to lead to an exception throw *unless*
  // either the configuration has been set to match all events, or the
  // configuration is set to use specific process names.
  loadTriggerResults(ev);

  return any_of(begin(p_and_e_selectors_),
                end(p_and_e_selectors_),
                [](auto& s) { return s.match(); });
}

art::Handle<art::TriggerResults>
art::detail::CachedProducts::getOneTriggerResults(Event const& ev) const
{
  const_cast<CachedProducts*>(this)->loadTriggerResults(ev);
  if (numberFound_ == 1) {
    return p_and_e_selectors_[0].triggerResults();
  }
  if (numberFound_ == 0) {
    throw art::Exception(art::errors::ProductNotFound, "TooFewProducts")
      << "CachedProducts::getOneTriggerResults: "
      << "too few products found, exepcted one, got zero\n";
  }
  throw art::Exception(art::errors::ProductNotFound, "TooManyMatches")
    << "CachedProducts::getOneTriggerResults: "
    << "too many products found, expected one, got " << numberFound_ << '\n';
}

void
art::detail::CachedProducts::clearTriggerResults()
{
  for_all(p_and_e_selectors_, [](auto& p) { p.clearTriggerResults(); });
  loadDone_ = false;
  numberFound_ = 0;
}

void
art::detail::CachedProducts::loadTriggerResults(Event const& ev)
{
  // Get all the TriggerResults objects for the process names we are
  // interested in.
  if (loadDone_) {
    return;
  }
  loadDone_ = true;
  // Note: The loadTriggerResults call might throw, so numberFound_
  // may be less than expected.
  for_all(p_and_e_selectors_, [&, this](auto& s) {
    s.loadTriggerResults(ev);
    ++numberFound_;
  });
}
