// ======================================================================
//
// EngineCreator - enable a derived class to access the
//                 RandomNumberGenerator::createEngine()
//
// ======================================================================

#include "art/Framework/Core/detail/EngineCreator.h"
#include "art/Utilities/bold_fontify.h"
#include "canvas/Utilities/Exception.h"
#include "cetlib_except/demangle.h"
#include "fhiclcpp/ParameterSet.h"

#include <utility>

using art::detail::EngineCreator;
using fhicl::ParameterSet;

EngineCreator::EngineCreator(std::string const& moduleLabel,
                             ScheduleID const sid)
  : moduleLabel_{moduleLabel}, sid_{sid}
{}

EngineCreator::base_engine_t&
EngineCreator::createEngine(seed_t const seed)
{
  return createEngine(seed, rng()->defaultEngineKind());
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(seed_t const seed,
                            std::string const& kind_of_engine_to_make)
{
  requireValid();
  return rng()->createEngine(sid_, moduleLabel_, seed, kind_of_engine_to_make);
}

EngineCreator::base_engine_t&
EngineCreator::createEngine(seed_t const seed,
                            std::string const& kind_of_engine_to_make,
                            label_t const& engine_label)
{
  requireValid();
  return rng()->createEngine(
    sid_, moduleLabel_, seed, kind_of_engine_to_make, engine_label);
}

art::ServiceHandle<art::RandomNumberGenerator>&
EngineCreator::rng()
{
  static art::ServiceHandle<art::RandomNumberGenerator> rng;
  return rng;
}

void
EngineCreator::requireValid()
{
  if (sid_.isValid() && !moduleLabel_.empty()) {
    return;
  }
  throw Exception{
    errors::LogicError,
    "An error occurred while creating a random-number engine.\n\n"}
    << "No module label or schedule ID available to create engine.\n"
    << "Please ensure that your module calls the correct base-class\n"
       "constructor.  The module class and module label can be determined\n"
       "by looking for the '<module class>:<module label>@Construction' "
       "string\n"
       "in the message context a few lines above.  For example, if your\n"
       "module is a filter with class name 'MyFilter' please make the\n"
       "following change to your constructor:\n\n"
    << "  Wrong:  MyFilter(ParameterSet const& ps) :\n"
    << "            dataMembers_, ...\n"
    << "          {}\n\n"
    << "  Right:  MyFilter(ParameterSet const& ps) :\n"
    << "            " << art::detail::bold_fontify("art::EDFilter{ps},")
    << " dataMembers_, ...\n"
    << "          {}\n\n"
    << "If your module is a producer, 'art::EDFilter{ps}' above should be\n"
       "replaced with 'art::EDProducer{ps}.\n";
}

// ======================================================================
