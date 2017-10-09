#ifndef art_Framework_Core_EmptyEventTimestampPlugin_h
#define art_Framework_Core_EmptyEventTimestampPlugin_h

////////////////////////////////////////////////////////////////////////
// EmptyEventTimestampPlugin
//
// The base class for user-defined timestamp generating plugins specific
// to EmptyEvent.
//
// TimestampPlugin: { plugin_type: <pluginClassName> ... }
//
// Note that there is currently no use case for allowing multiple
// plugins in a given job.
//
// Entry points are called as indicated by their names, with the
// following extra notes:
//
// * doEventTimeStamp() will always be called prior to
//   doBeginRunTimestamp() and friends, allowing a default
//   implementation of (e.g) beginRunTimestamp() to use the last
//   generated event timestamp.
//
// * doRewind() is called when EmptyEvent is asked to rewind to its
//   initial conditions.
//
// General notes.
//
// * Subclasses implementing this interface *must* implement
//   eventTimestamp() and rewind(). Other entry points are optional.
//
// * Subclasses should not provide a header file: any communication with
//   the plugin is accomplished solely via the base class interface.
//
// * Use the macro DEFINE_EMPTYEVENTTIMESTAMP_PLUGIN(<classname>) (see
//   below) at the bottom of your implementation file to declare your
//   plugin to the art system.
//
////////////////////////////////////////////////////////////////////////

#include "art/Utilities/BasicPluginMacros.h"
#include "canvas/Persistency/Provenance/Timestamp.h"
#include "cetlib/PluginTypeDeducer.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

#define DEFINE_ART_EMPTYEVENTTIMESTAMP_PLUGIN(klass)                           \
  CET_PROVIDE_FILE_PATH()                                                      \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(klass)                                   \
  DEFINE_BASIC_PLUGIN(klass, art::EmptyEventTimestampPlugin)

namespace art {
  class EmptyEventTimestampPlugin;

  // Forward declarations.
  class Event;
  class EventID;
  class Run;
  class RunID;
  class SubRun;
  class SubRunID;
} // namespace art

namespace cet {
  template <>
  struct PluginTypeDeducer<art::EmptyEventTimestampPlugin> {
    static std::string const value;
  };
} // namespace cet

class art::EmptyEventTimestampPlugin {
public:
  EmptyEventTimestampPlugin(fhicl::ParameterSet const& pset);

  void doBeginJob();
  void doEndJob();
  void doBeginRun(Run const& r);
  Timestamp doBeginRunTimestamp(RunID const& rid);
  void doBeginSubRun(SubRun const& sr);
  Timestamp doBeginSubRunTimestamp(SubRunID const& srid);

  Timestamp doEventTimestamp(EventID const& e);

  void doRewind();

  virtual ~EmptyEventTimestampPlugin() = default;

private:
  Timestamp lastEventTimestamp_;

  virtual void beginJob(){};
  virtual void endJob(){};
  virtual void beginRun(Run const&){};
  virtual Timestamp
  beginRunTimestamp(RunID const&)
  {
    return lastEventTimestamp_;
  };
  virtual void beginSubRun(SubRun const&){};
  virtual Timestamp
  beginSubRunTimestamp(SubRunID const&)
  {
    return lastEventTimestamp_;
  };

  virtual Timestamp eventTimestamp(EventID const&) = 0;

  virtual void rewind() = 0;
};
#endif /* art_Framework_Core_EmptyEventTimestampPlugin_h */

inline void
art::EmptyEventTimestampPlugin::doBeginJob()
{
  beginJob();
}

inline void
art::EmptyEventTimestampPlugin::doEndJob()
{
  endJob();
}

inline void
art::EmptyEventTimestampPlugin::doBeginRun(Run const& r)
{
  beginRun(r);
}

inline art::Timestamp
art::EmptyEventTimestampPlugin::doBeginRunTimestamp(RunID const& rid)
{
  return beginRunTimestamp(rid);
}

inline void
art::EmptyEventTimestampPlugin::doBeginSubRun(SubRun const& sr)
{
  beginSubRun(sr);
}

inline art::Timestamp
art::EmptyEventTimestampPlugin::doBeginSubRunTimestamp(SubRunID const& srid)
{
  return beginSubRunTimestamp(srid);
}

inline art::Timestamp
art::EmptyEventTimestampPlugin::doEventTimestamp(EventID const& eid)
{
  lastEventTimestamp_ = eventTimestamp(eid);
  return lastEventTimestamp_;
}

inline void
art::EmptyEventTimestampPlugin::doRewind()
{
  rewind();
}

// Local Variables:
// mode: c++
// End:
