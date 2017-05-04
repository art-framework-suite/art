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
art::detail::CachedProducts::setup(vector<pair<string, string>> const& path_specs,
                                   vector<string> const& triggernames,
                                   string const& process_name)
{
  // Turn the passed path specs into a map of process name to
  // a vector of trigger names.
  map<string, vector<string>> paths_for_process;
  for (auto i = path_specs.begin(), e = path_specs.end(); i != e; ++i) {
    if (i->second == "") {
      // Use passed process name if none specified in the path spec.
      paths_for_process[process_name].push_back(i->first);
      continue;
    }
    paths_for_process[i->second].push_back(i->first);
  }
  // Now go through all the process names found, and create an
  // event selector for each one.
  for (auto i = paths_for_process.begin(), e = paths_for_process.end();
       i != e; ++i) {
    if (i->first == process_name) {
      // For the passed process name we have been given the trigger names.
      p_and_e_selectors_.emplace_back(i->first, EventSelector{i->second, triggernames});
      continue;
    }
    // For other process names we do not know the trigger names.
    p_and_e_selectors_.emplace_back(i->first, EventSelector{i->second});
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
                [](auto& s){ return s.match(); });
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
    << "too many products found, expected one, got "
    << numberFound_
    << '\n';
}

void
art::detail::CachedProducts::clearTriggerResults()
{
  for_all(p_and_e_selectors_, [](auto& p){ p.clearTriggerResults(); });
  loadDone_ = false;
  numberFound_ = 0;
}

void
art::detail::CachedProducts::loadTriggerResults(Event const& ev)
{
  // Get all the TriggerResults objects for the process names we are
  // interested in.
  if (loadDone_) { return; }
  loadDone_ = true;
  // Note: The loadTriggerResults call might throw, so numberFound_
  // may be less than expected.
  for_all(p_and_e_selectors_,
          [&,this](auto& s){ s.loadTriggerResults(ev); ++numberFound_;});
}
