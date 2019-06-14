#ifndef art_Framework_Core_FileCatalogMetadataPlugin_h
#define art_Framework_Core_FileCatalogMetadataPlugin_h
// vim: set sw=2 expandtab :

//
//  The base class for user-defined plugins to produce SAM metadata
//  specific to a particular output stream. Configure with:
//
//  FCMDPlugins: [ { plugin_type: <pluginClassName> ... }, ... ]
//
//  Entry points are called as indicated by their names, with the
//  following extra notes:
//
//  * doCollectMetadata(...) is called only when an event is selected for
//    output by the output module for which the current plugin is
//    configured.
//
//  * doProduceMetadata() is called immediately before a file is closed
//    by the output module for which the current plugin is configured.
//
//  General notes.
//
//  * Subclasses implementing this interface *must* implement
//    produceMetadata(). Other entry points are optional.
//
//  * Subclasses should not provide a header file: any communication with
//    the plugin is accomplished solely via the base class interface.
//
//  * Use the  macro DEFINE_FILECATALOGMETADATA_PLUGIN(<classname>) (see
//    below) at the bottom of your implementation file to declare your
//    plugin to the art system.
//

#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "art/Utilities/BasicPluginMacros.h"
#include "cetlib/PluginTypeDeducer.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"

#define DEFINE_ART_FILECATALOGMETADATA_PLUGIN(klass)                           \
  CET_PROVIDE_FILE_PATH()                                                      \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(klass)                                   \
  DEFINE_BASIC_PLUGIN(klass, art::FileCatalogMetadataPlugin)

namespace art {

  class Event;
  class FileCatalogMetadataPlugin;
  class Run;
  class SubRun;

} // namespace art

namespace cet {

  template <>
  struct PluginTypeDeducer<art::FileCatalogMetadataPlugin> {
    static std::string const value;
  };

} // namespace cet

namespace art {

  class FileCatalogMetadataPlugin {
  public:
    using collection_type = FileCatalogMetadata::collection_type;

    FileCatalogMetadataPlugin(fhicl::ParameterSet const& pset);
    virtual ~FileCatalogMetadataPlugin() = default;

    void doBeginJob();
    void doEndJob();

    void doBeginRun(Run const& r);
    void doEndRun(Run const& r);

    void doBeginSubRun(SubRun const& sr);
    void doEndSubRun(SubRun const& sr);

    void doCollectMetadata(Event const& e);

    collection_type doProduceMetadata();

  private:
    virtual void beginJob(){}
    virtual void endJob(){}

    virtual void beginRun(Run const&){}
    virtual void endRun(Run const&){}

    virtual void beginSubRun(SubRun const&){}
    virtual void endSubRun(SubRun const&){}

    virtual void collectMetadata(Event const&){}

    virtual collection_type produceMetadata() = 0;
  };

  inline void
  FileCatalogMetadataPlugin::doBeginJob()
  {
    beginJob();
  }

  inline void
  FileCatalogMetadataPlugin::doEndJob()
  {
    endJob();
  }

  inline void
  FileCatalogMetadataPlugin::doCollectMetadata(Event const& e)
  {
    collectMetadata(e);
  }

  inline void
  FileCatalogMetadataPlugin::doBeginRun(Run const& r)
  {
    beginRun(r);
  }

  inline void
  FileCatalogMetadataPlugin::doEndRun(Run const& r)
  {
    endRun(r);
  }

  inline void
  FileCatalogMetadataPlugin::doBeginSubRun(SubRun const& r)
  {
    beginSubRun(r);
  }

  inline void
  FileCatalogMetadataPlugin::doEndSubRun(SubRun const& r)
  {
    endSubRun(r);
  }

  inline auto
  FileCatalogMetadataPlugin::doProduceMetadata() -> collection_type
  {
    return produceMetadata();
  }

} // namespace art

#endif /* art_Framework_Core_FileCatalogMetadataPlugin_h */

// Local Variables:
// mode: c++
// End:
