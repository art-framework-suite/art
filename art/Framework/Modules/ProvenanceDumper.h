#ifndef art_Framework_Modules_ProvenanceDumper_h
#define art_Framework_Modules_ProvenanceDumper_h
////////////////////////////////////////////////////////////////////////
// ProvenanceDumper
//
// This class template is used to create output modules capable of
// iterating over all Provenances in an event, subrun or run.
//
// Note that it is always possible to obtain the provenance for a
// *particular* product through its handle: this class template is only
// for cases where one wishes to iterate over many Provenances without
// necesarily loading each product.
//
// There are two parameters that control the behavior of this module
// template:
//
//    1. wantPresentOnly (bool, default true)
//
//       If true, produce provenances and invoke relevant callbacks only
//       if the product is actually present.
//
//    2. resolveProducts (bool, default true).
//
//       If true, attempt to resolve the product before attempting to
//       determine whether it is present.
//
// The ProvenanceDumper class template requires the use of a type DETAIL
// as its template paramenter; this type DETAIL must supply the
// following non-static member functions:
//
//    DETAIL(art::OutputModule::Table<Config> const &p);
//
//    // Construct an object of type DETAIL. The 'Config' struct
//    // will be that provided corresponding to the module constructing the
//    // DETAIL. It is recommended (but not enforced) that detail
//    // parameters be placed in their own (eg "detail") ParameterSet to
//    // reduce the potential for clashes with parameters used by the
//    // template.
//
// In addition, T may optionally provide any or all of the following
// member functions; each will be callled at the appropriate time iff it
// is declared. Note that *none* of the functions here should be
// declared const.
//
//    void beginJob();
//
//    // Called at the beginning of the job, from the template module's
//    // beginJob() member function.
//
//    void preProcessEvent();
//
//    // Called prior to looping over any Provenances to be found in the
//    // event (always called if present)..
//
//    void processEventProvenance(art::Provenance const &);
//
//    // Called for each Provenance in the event (not called if there
//    // are none).
//
//    void postProcessEvent();
//
//    // Called after looping over any Provenances to be found in the
//    // event (always called if present)..
//
//    void preProcessSubRun();
//
//    // Called prior to looping over any Provenances to be found in the
//    // subrun (always called if present)..
//
//    void processSubRunProvenance(art::Provenance const &);
//
//    // Called for each Provenance in the subrun (not called if there
//    // are none).
//
//    void postProcessSubRun();
//
//    // Called after looping over any Provenances to be found in the
//    // subrun (always called if present)..
//
//    void preProcessRun();
//
//    // Called prior to looping over any Provenances to be found in the
//    // run (always called if present)..
//
//    void processRunProvenance(art::Provenance const &);
//
//    // Called for each Provenance in the run (not called if there
//    // are none).
//
//    void postProcessRun();
//
//    // Called after looping over any Provenances to be found in the
//    // run (always called if present)..
//
//    void endJob();
//
//    // Called at the end of the job, from the template module's
//    // endJob() member function.
//
////////////////////////////////////
// Advanced notes.
//
// For those interested in how the framework obtains the provenance
// information, here are some notes:
//
// This module template contains a lot of template metaprograming to
// ensure that only those functions relevant to the user's purpose (and
// therefore defined) are called, and at the right time. If you are
// interested in how the provenance is obtained, the only function
// necessary to this understanding below is:
//
//    template <typename DETAIL>
//    template <typename PRINCIPAL>
//    void
//    art::detail::PrincipalProcessor<DETAIL>::
//    operator()(PRINCIPAL const & p,
//               void (DETAIL:: *func)(art::Provenance const &)) const
//
// This function will loop over the Groups in a particular Principal,
// attempt to resolve the product if possible and then construct a
// Provenance to pass to the correct callback function func.
//
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Core/OutputModule.h"
#include "art/Framework/Modules/detail/ProvenanceDumperImpl.h"
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/ConfigTable.h"
#include "canvas/Utilities/Exception.h"
#include "canvas/Utilities/detail/metaprogramming.h"
#include "cetlib/exempt_ptr.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/types/Sequence.h"

#include <algorithm>

namespace art {
  template <typename DETAIL, typename Enable = void> class ProvenanceDumper;

  template <typename DETAIL, typename Enable = void>
  struct ProvenanceDumperConfig {
    fhicl::TableFragment<art::OutputModule::Config> omConfig;
    fhicl::Atom<bool> wantPresentOnly { fhicl::Name("wantPresentOnly"), true };
    fhicl::Atom<bool> resolveProducts { fhicl::Name("resolveProducts"), true };
  };

  template <typename DETAIL>
  struct ProvenanceDumperConfig<DETAIL, typename detail::enable_if_type<typename DETAIL::Config>::type> {
    fhicl::TableFragment<art::OutputModule::Config> omConfig;
    fhicl::Atom<bool> wantPresentOnly { fhicl::Name("wantPresentOnly"), true };
    fhicl::Atom<bool> resolveProducts { fhicl::Name("resolveProducts"), true };
    fhicl::TableFragment<typename DETAIL::Config> user;
  };

  namespace detail {
    template <typename DETAIL>
    class PrincipalProcessor;
  }
}

template <typename DETAIL, typename Enable>
class art::ProvenanceDumper : public OutputModule {
public:

  explicit ProvenanceDumper(fhicl::ParameterSet const & ps)
    :
    OutputModule{ps},
    detail_{ps},
    wantPresentOnly_{ps.get<bool>("wantPresentOnly", true)},
    resolveProducts_{ps.get<bool>("resolveProducts", true)},
    pp_{detail_, wantPresentOnly_, resolveProducts_},
    impl_{detail_, pp_}
  {}

private:

  void beginJob() override { impl_.beginJob(); }
  void endJob()  override { impl_.endJob(); }

  void write      (EventPrincipal  & e ) override { impl_.write(e); }
  void writeSubRun(SubRunPrincipal & sr) override { impl_.writeSubRun(sr); }
  void writeRun   (RunPrincipal    & r ) override { impl_.writeRun(r); }

  DETAIL detail_;
  bool wantPresentOnly_;
  bool resolveProducts_;
  detail::PrincipalProcessor<DETAIL> pp_;
  detail::ProvenanceDumperImpl<DETAIL> impl_;

};


namespace art {

  template <typename DETAIL>
  class ProvenanceDumper<DETAIL, typename art::detail::enable_if_type<typename DETAIL::Config>::type> :
    public OutputModule {
  public:

    using Parameters = art::ConfigTable<ProvenanceDumperConfig<DETAIL>,
                                        art::OutputModule::Config::KeysToIgnore>;

    explicit ProvenanceDumper(Parameters const & ps)
      :
      OutputModule{ps().omConfig, ps.get_PSet()},
      detail_{ps().user},
      wantPresentOnly_{ps().wantPresentOnly()},
      resolveProducts_{ps().resolveProducts()},
      pp_{detail_, wantPresentOnly_, resolveProducts_},
      impl_{detail_, pp_}
    {}

  private:

    void beginJob() override { impl_.beginJob(); }
    void endJob()  override { impl_.endJob(); }

    void write      (EventPrincipal  & e ) override { impl_.write(e); }
    void writeSubRun(SubRunPrincipal & sr) override { impl_.writeSubRun(sr); }
    void writeRun   (RunPrincipal    & r ) override { impl_.writeRun(r); }

    DETAIL detail_;
    bool wantPresentOnly_;
    bool resolveProducts_;
    detail::PrincipalProcessor<DETAIL> pp_;
    detail::ProvenanceDumperImpl<DETAIL> impl_;

  };
}

#endif /* art_Framework_Modules_ProvenanceDumper_h */

// Local Variables:
// mode: c++
// End:
