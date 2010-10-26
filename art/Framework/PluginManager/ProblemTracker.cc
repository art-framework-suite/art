

#include "art/Framework/PluginManager/ProblemTracker.h"
#include "art/Framework/PluginManager/PluginManager.h"
#include "art/Framework/PluginManager/standard.h"

#include <string>

namespace art
{

  // -----------------------------------------------

  bool ProblemTracker::dead_ = true;
  //artplugin::DebugAids::AssertHook ProblemTracker::old_assert_hook_ = 0;

  ProblemTracker::ProblemTracker()
  {
    dead_ = false;
    //old_assert_hook_ = artplugin::DebugAids::failHook(&failure);
    if(not artplugin::PluginManager::isAvailable()) {
      artplugin::PluginManager::Config config(artplugin::standard::config());

      artplugin::PluginManager::configure(config);
    }
  }

  ProblemTracker::~ProblemTracker()
  {
    // since this is a singleton, we will not restore the old handle
    dead_ = true;
  }

  ProblemTracker* ProblemTracker::instance()
  {
    static ProblemTracker pt;
    return &pt;
  }

  // ---------------------------------------------

  AssertHandler::AssertHandler():pt_(ProblemTracker::instance())
  {
  }

  AssertHandler::~AssertHandler()
  {
  }


}
