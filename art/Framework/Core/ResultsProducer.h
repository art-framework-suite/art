#ifndef art_Framework_Core_ResultsProducer_h
#define art_Framework_Core_ResultsProducer_h

#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/RPWorkerT.h"
#include "art/Utilities/BasicHelperMacros.h"
#include "cetlib/PluginTypeDeducer.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  class ResultsProducer;

  class Event;
  class Results;
  class Run;
  class SubRun;
}

#include <memory>
#include <string>

namespace cet {
  template <> struct PluginTypeDeducer<art::ResultsProducer> {
    static std::string const value;
  };
}

class art::ResultsProducer : ProductRegistryHelper {
protected:
  // This is the only base class function that subclasses should call.
  using ProductRegistryHelper::produces;

public:
  virtual ~ResultsProducer() = default;

  void doBeginJob();
  void doEndJob();
  void doBeginSubRun(SubRun const &);
  void doEndSubRun(SubRun const &);
  void doBeginRun(Run const &);
  void doEndRun(Run const &);
  void doEvent(Event const &);
  void doReadResults(Results const &);
  void doWriteResults(Results &);
  void doClear();

  void registerProducts(MasterProductRegistry & mpr,
                        ModuleDescription const & md)
    {
      ProductRegistryHelper::registerProducts(mpr, md);
    }

private:
  // Functions for implementation by subclasses.
  virtual void beginJob();
  virtual void endJob();

  virtual void beginSubRun(SubRun const &);
  virtual void endSubRun(SubRun const &);

  virtual void beginRun(Run const &);
  virtual void endRun(Run const &);

  virtual void event(Event const &);

  virtual void readResults(Results const &);
  virtual void writeResults(Results &) = 0;

  virtual void clear() = 0;
};

inline
void
art::ResultsProducer::
doBeginJob()
{
  beginJob();
}

inline
void
art::ResultsProducer::
doEndJob()
{
  endJob();
}

inline
void
art::ResultsProducer::
doBeginSubRun(SubRun const & sr)
{
  beginSubRun(sr);
}

inline
void
art::ResultsProducer::
doEndSubRun(SubRun const & sr)
{
  endSubRun(sr);
}

inline
void
art::ResultsProducer::
doBeginRun(Run const & r)
{
  beginRun(r);
}

inline
void
art::ResultsProducer::
doEndRun(Run const & r)
{
  endRun(r);
}

inline
void
art::ResultsProducer::
doEvent(Event const & e)
{
  event(e);
}

inline
void
art::ResultsProducer::
doReadResults(Results const & res)
{
  readResults(res);
}

inline
void
art::ResultsProducer::
doClear()
{
  clear();
}

#define DEFINE_ART_RESULTS_PLUGIN(klass)                           \
  PROVIDE_FILE_PATH()                                              \
  PROVIDE_DESCRIPTION(klass)                                       \
  DEFINE_BASIC_PLUGINTYPE_FUNC(art::ResultsProducer)               \
  extern "C" {                                                     \
    std::unique_ptr<art::RPWorker>                            \
    makeRP(art::RPParams const & rpParams,                         \
           fhicl::ParameterSet const & ps)                         \
    {                                                              \
      return                                                       \
        std::make_unique<art::RPWorkerT<klass>>(rpParams, ps);     \
    }                                                              \
  }


#endif /* art_Framework_Core_ResultsProducer_h */

// Local Variables:
// mode: c++
// End:
