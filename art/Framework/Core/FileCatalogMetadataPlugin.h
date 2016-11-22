#ifndef art_Framework_Core_FileCatalogMetadataPlugin_h
#define art_Framework_Core_FileCatalogMetadataPlugin_h

////////////////////////////////////////////////////////////////////////
// FileCatalogMetadataPlugin
//
// The base class for user-defined plugins to produce SAM metadata
// specific to a particular output stream. Configure with:
//
// FCMDPlugins: [ { plugin_type: <pluginClassName> ... }, ... ]
//
// Entry points are called as indicated by their names, with the
// following extra notes:
//
// * doCollectMetadata(...) is called only when an event is selected for
//   output by the output module for which the current plugin is
//   configured.
//
// * doProduceMetadata() is called immediately before a file is closed
//   by the output module for which the current plugin is configured.
//
// General notes.
//
// * Subclasses implementing this interface *must* implement
//   produceMetadata(). Other entry points are optional.
//
// * Subclasses should not provide a header file: any communication with
//   the plugin is accomplished solely via the base class interface.
//
// * Use the  macro DEFINE_FILECATALOGMETADATA_PLUGIN(<classname>) (see
//   below) at the bottom of your implementation file to declare your
//   plugin to the art system.
//
////////////////////////////////////////////////////////////////////////

#include "art/Utilities/BasicHelperMacros.h"
#include "art/Utilities/BasicPluginMacros.h"
#include "cetlib/PluginTypeDeducer.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "fhiclcpp/ParameterSet.h"

#define DEFINE_ART_FILECATALOGMETADATA_PLUGIN(klass)  \
  PROVIDE_FILE_PATH()                                 \
  PROVIDE_ALLOWED_CONFIGURATION(klass)                          \
  DEFINE_BASIC_PLUGIN(klass,                          \
                      art::FileCatalogMetadataPlugin)

namespace art {
  class FileCatalogMetadataPlugin;

  // Forward declarations.
  class Event;
  class Run;
  class SubRun;
}

namespace cet {
  template <> struct PluginTypeDeducer<art::FileCatalogMetadataPlugin> {
    static std::string const value;
  };
}

class art::FileCatalogMetadataPlugin {
public:
  typedef art::FileCatalogMetadata::collection_type collection_type;

  FileCatalogMetadataPlugin(fhicl::ParameterSet const & pset);

  void doBeginJob();
  void doEndJob();
  void doCollectMetadata(Event const & e);
  void doBeginRun(Run const & r);
  void doEndRun(Run const & r);
  void doBeginSubRun(SubRun const & sr);
  void doEndSubRun(SubRun const & sr);
  collection_type doProduceMetadata();

  virtual ~FileCatalogMetadataPlugin() = default;

private:
  virtual void beginJob() { };
  virtual void endJob() { };
  virtual void collectMetadata(Event const &) { };
  virtual void beginRun(Run const &) { };
  virtual void endRun(Run const &) { };
  virtual void beginSubRun(SubRun const &) { };
  virtual void endSubRun(SubRun const &) { };
  virtual collection_type produceMetadata() = 0;
};
#endif /* art_Framework_Core_FileCatalogMetadataPlugin_h */

inline
void
art::FileCatalogMetadataPlugin::
doBeginJob()
{
  beginJob();
}

inline
void
art::FileCatalogMetadataPlugin::
doEndJob()
{
  endJob();
}

inline
void
art::FileCatalogMetadataPlugin::
doCollectMetadata(Event const & e)
{
  collectMetadata(e);
}

inline
void
art::FileCatalogMetadataPlugin::
doBeginRun(Run const & r)
{
  beginRun(r);
}

inline
void
art::FileCatalogMetadataPlugin::
doEndRun(Run const & r)
{
  endRun(r);
}

inline
void
art::FileCatalogMetadataPlugin::
doBeginSubRun(SubRun const & r)
{
  beginSubRun(r);
}

inline
void
art::FileCatalogMetadataPlugin::
doEndSubRun(SubRun const & r)
{
  endSubRun(r);
}

inline
auto
art::FileCatalogMetadataPlugin::
doProduceMetadata()
-> collection_type
{
  return produceMetadata();
}

// Local Variables:
// mode: c++
// End:
