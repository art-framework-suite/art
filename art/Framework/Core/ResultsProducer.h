#ifndef art_Framework_Core_ResultsProducer_h
#define art_Framework_Core_ResultsProducer_h
// vim: set sw=2 expandtab :

//
//  Base class for all ResultsProducer plugins.
//
//  Subclasses shuld not have a separate header, as, in common with other
//  art modules such as EDProducers and EDAnalyzers, communiation between
//  modules is only allowed via defined data products and not via calls
//  to the class' interface.
//
//  Notes for subclass implementors:
//
//  * Subclass implementations *must* invoke:
//
//    DEFINE_ART_RESULTS_PLUGIN(class)
//
//  * Subclasses *must* define:
//
//    Constructor(fhicl::ParameterSet const&);
//
//      Declare data products to be put into the Results (via
//      art::Results::put() in writeResults() below) using produces<>(),
//      in a similar fashion to producers and filters.
//
//    void writeResults(art::Results&);
//
//      Called immediately prior to output file closure. Users should
//      put() their declared products into the Results object. Using
//      getByLabel() here will access only those results products that were
//      put() by plugins executed earlier for the same output module. In
//      order to access results products from input files, use
//      readResults(), below. Note that for the purposes of product
//      retrieval, the "module label" of a results product is:
//
//           <output-module-label>#<results-producer-label>
//
//      For example for results producer label rp1 defined for output
//      module o1, any product produced by rp1 will have the label, o1#rp1.
//
//    void clear();
//
//      In this function (called after writeResults()), the user should
//      reset any accumulated information in preparation for (possibly)
//      accumulating data for the next output file.
//
//  * Subclasses *may* define:
//
//    void beginJob();
//
//    void endJob();
//
//    void beginSubRun(SubRun const &);
//
//    void endSubRun(SubRun const &);
//
//    void beginRun(Run const &);
//
//    void endRun(Run const &);
//
//    void event(Event const &);
//
//    void readResults(art::Results const &);
//
//      Access any results-level products in input files here. The user
//      is entirely responsible for combining information from possibly
//      multiple input files into a possible output product, and for
//      dealing with the fact that reading a product from an input here
//      is distinctly different from reading a product placed into the
//      outgoing results object by a ResultsProducer running in the same
//      job (which must be done in writeResults(), above).
//

#include "art/Framework/Core/ModuleBase.h"
#include "art/Framework/Core/ModuleType.h"
#include "art/Framework/Core/ProductRegistryHelper.h"
#include "art/Framework/Core/RPWorkerT.h"
#include "cetlib/PluginTypeDeducer.h"
#include "cetlib/ProvideFilePathMacro.h"
#include "cetlib/compiler_macros.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/AllowedConfigurationMacro.h"
#include "hep_concurrency/SerialTaskQueueChain.h"

#include <memory>
#include <set>
#include <string>

namespace art {

  class EventPrincipal;
  class ResultsPrincipal;
  class RunPrincipal;
  class SubRunPrincipal;

  class Event;
  class Results;
  class Run;
  class SubRun;

  class ResultsProducer : public ModuleBase, private ProductRegistryHelper {
  public: // MEMBER FUNCTIONS -- Special Member Functions
    virtual ~ResultsProducer() noexcept = default;

  protected:
    template <class P>
    void produces(std::string const& instanceName = {});

  public:
    void doBeginJob();
    void doEndJob();

    void doBeginRun(RunPrincipal const&);
    void doEndRun(RunPrincipal const&);

    void doBeginSubRun(SubRunPrincipal const&);
    void doEndSubRun(SubRunPrincipal const&);

    void doEvent(EventPrincipal const&);

    void doReadResults(ResultsPrincipal const&);
    void doWriteResults(ResultsPrincipal&);
    void doClear();

    void registerProducts(ProductDescriptions& producedProducts,
                          ModuleDescription const& md);

  private: // MEMBER FUNCTIONS -- API to be provided by derived classes
    virtual void readResults(Results const&);
    virtual void writeResults(Results&) = 0;

    virtual void clear() = 0;

    virtual void beginJob();
    virtual void endJob();

    virtual void beginRun(Run const&);
    virtual void endRun(Run const&);

    virtual void beginSubRun(SubRun const&);
    virtual void endSubRun(SubRun const&);

    virtual void event(Event const&);
  };

} // namespace art

template <class P>
inline void
art::ResultsProducer::produces(std::string const& instanceName)
{
  ProductRegistryHelper::produces<P, InResults>(instanceName);
}

namespace cet {
  template <>
  struct PluginTypeDeducer<art::ResultsProducer> {
    static std::string const value;
  };
} // namespace cet

#define DEFINE_ART_RESULTS_PLUGIN(klass)                                       \
  EXTERN_C_FUNC_DECLARE_START                                                  \
  CET_PROVIDE_FILE_PATH()                                                      \
  FHICL_PROVIDE_ALLOWED_CONFIGURATION(klass)                                   \
  DEFINE_BASIC_PLUGINTYPE_FUNC(art::ResultsProducer)                           \
  std::unique_ptr<art::RPWorker> makeRP(art::RPParams const& rpParams,         \
                                        fhicl::ParameterSet const& ps)         \
  {                                                                            \
    return std::make_unique<art::RPWorkerT<klass>>(rpParams, ps);              \
  }                                                                            \
  EXTERN_C_FUNC_DECLARE_END

#endif /* art_Framework_Core_ResultsProducer_h */

// Local Variables:
// mode: c++
// End:
