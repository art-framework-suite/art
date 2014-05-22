#ifndef art_Framework_Core_FileCatalogMetadataPlugin_h
#define art_Framework_Core_FileCatalogMetadataPlugin_h

#include "art/Utilities/BasicPluginMacros.h"
#include "cetlib/PluginTypeDeducer.h"
#include "art/Framework/Services/System/FileCatalogMetadata.h"
#include "fhiclcpp/ParameterSet.h"

#define DEFINE_FILECATALOGMETADATA_PLUGIN(klass)      \
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
