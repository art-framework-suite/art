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
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Group.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "art/Utilities/Exception.h"
#include "art/Utilities/detail/metaprogramming.h"
#include "cetlib/exempt_ptr.h"
#include "cpp0x/algorithm"

#include "fhiclcpp/types/Atom.h"
#include "fhiclcpp/ParameterSet.h"
#include "fhiclcpp/types/Sequence.h"

namespace art {
  template <typename DETAIL> class ProvenanceDumper;

  struct ProvenanceDumperConfig {
    fhicl::Atom<bool> wantPresentOnly { fhicl::Name("wantPresentOnly"), true };
    fhicl::Atom<bool> resolveProducts { fhicl::Name("resolveProducts"), true };
  };

  namespace detail {
    template <typename DETAIL>
    class PrincipalProcessor;
  }
}

template <typename DETAIL>
class art::ProvenanceDumper : public OutputModule {
public:

  using Parameters = art::OutputModule::Table<typename DETAIL::Config>;
  explicit ProvenanceDumper(fhicl::ParameterSet const & ps);
  virtual ~ProvenanceDumper();

private:
  virtual void beginJob();
  virtual void
  write(EventPrincipal const & e);
  virtual void
  writeRun(RunPrincipal const & r);
  virtual void
  writeSubRun(SubRunPrincipal const & sr);
  virtual void endJob();

  DETAIL detail_;
  bool wantPresentOnly_;
  bool resolveProducts_;
  detail::PrincipalProcessor<DETAIL> pp_;

};



template <typename DETAIL>
inline
art::ProvenanceDumper<DETAIL>::
ProvenanceDumper(fhicl::ParameterSet const & ps)
  :
  OutputModule(art::OutputModule::Table<typename DETAIL::Config>(ps)),
  detail_(ps),
  wantPresentOnly_(ps.get<bool>("wantPresentOnly", true)),
  resolveProducts_(ps.get<bool>("resolveProducts", true)),
  pp_(detail_, wantPresentOnly_, resolveProducts_)
{
}

template <typename DETAIL>
art::ProvenanceDumper<DETAIL>::
~ProvenanceDumper()
{
}

namespace art {
  namespace detail {
    template <typename DETAIL>
    class PrincipalProcessor {
    public:
      PrincipalProcessor(DETAIL & detail,
                         bool wantPresentOnly,
                         bool resolveProducts)
        :
        detail_(detail),
        wantPresentOnly_(wantPresentOnly),
        resolveProducts_(resolveProducts)
      { }
      template <typename PRINCIPAL>
      void
      operator()(PRINCIPAL const & p,
                 void (DETAIL:: *func)(art::Provenance const &)) const;
    private:
      DETAIL & detail_;
      bool wantPresentOnly_;
      bool resolveProducts_;
    };
  }
}

// The function that does all the work: everything else is fluff.
template <typename DETAIL>
template <typename PRINCIPAL>
void
art::detail::PrincipalProcessor<DETAIL>::
operator()(PRINCIPAL const & p,
           void (DETAIL:: *func)(art::Provenance const &)) const
{
  if (!p.size()) { return; } // Nothing to do.
  for (typename PRINCIPAL::const_iterator
       it  = p.begin(),
       end = p.end();
       it != end;
       ++it) {
    Group const & g = *(it->second);
    if (wantPresentOnly_ && resolveProducts_) {
      try {
        if (!g.resolveProduct(false, g.producedWrapperType()))
        { throw Exception(errors::DataCorruption, "data corruption"); }
      }
      catch (art::Exception const & e) {
        if (e.category() != "ProductNotFound")
        { throw; }
        if (g.anyProduct())
          throw art::Exception(errors::LogicError, "ProvenanceDumper", e)
              << "Product reported as not present, but is pointed to nonetheless!";
      }
    }
    if ((!wantPresentOnly_) || g.anyProduct()) {
      (detail_.*func)(Provenance(cet::exempt_ptr<Group const>(&g)));
    }
  }
}

namespace art {
  namespace detail {
    ////////////////////////////////////////////////////////////////////////
    // Metaprogramming to deal with optional pre/post and begin/end job functions.
    template <typename DETAIL, void (DETAIL:: *)()> struct detail_void_function;

    template <typename DETAIL>
    struct do_not_call_detail_void_function {
      do_not_call_detail_void_function(DETAIL &) { }
    };

    // void DETAIL::beginJob();
    template <typename DETAIL> no_tag has_detail_beginJob_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_beginJob_function_helper(detail_void_function<DETAIL, &DETAIL::beginJob> *);
    template <typename DETAIL> struct has_detail_beginJob_function {
      static bool const value =
        sizeof(has_detail_beginJob_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    struct call_detail_beginJob_function {
      call_detail_beginJob_function(DETAIL & detail) { detail.beginJob(); }
    };
    // void DETAIL::preProcessEvent();
    template <typename DETAIL> no_tag has_detail_preEvent_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_preEvent_function_helper(detail_void_function<DETAIL, &DETAIL::preProcessEvent> *);
    template <typename DETAIL> struct has_detail_preEvent_function {
      static bool const value =
        sizeof(has_detail_preEvent_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    struct call_detail_preEvent_function {
      call_detail_preEvent_function(DETAIL & detail) { detail.preProcessEvent(); }
    };
    // void DETAIL::postProcessEvent();
    template <typename DETAIL> no_tag has_detail_postEvent_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_postEvent_function_helper(detail_void_function<DETAIL, &DETAIL::postProcessEvent> *);
    template <typename DETAIL> struct has_detail_postEvent_function {
      static bool const value =
        sizeof(has_detail_postEvent_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    struct call_detail_postEvent_function {
      call_detail_postEvent_function(DETAIL & detail) { detail.postProcessEvent(); }
    };
    // void DETAIL::preProcessSubRun();
    template <typename DETAIL> no_tag has_detail_preSubRun_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_preSubRun_function_helper(detail_void_function<DETAIL, &DETAIL::preProcessSubRun> *);
    template <typename DETAIL> struct has_detail_preSubRun_function {
      static bool const value =
        sizeof(has_detail_preSubRun_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    struct call_detail_preSubRun_function {
      call_detail_preSubRun_function(DETAIL & detail) { detail.preProcessSubRun(); }
    };
    // void DETAIL::postProcessSubRun();
    template <typename DETAIL> no_tag has_detail_postSubRun_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_postSubRun_function_helper(detail_void_function<DETAIL, &DETAIL::postProcessSubRun> *);
    template <typename DETAIL> struct has_detail_postSubRun_function {
      static bool const value =
        sizeof(has_detail_postSubRun_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    struct call_detail_postSubRun_function {
      call_detail_postSubRun_function(DETAIL & detail) { detail.postProcessSubRun(); }
    };
    // void DETAIL::preProcessRun();
    template <typename DETAIL> no_tag has_detail_preRun_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_preRun_function_helper(detail_void_function<DETAIL, &DETAIL::preProcessRun> *);
    template <typename DETAIL> struct has_detail_preRun_function {
      static bool const value =
        sizeof(has_detail_preRun_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    struct call_detail_preRun_function {
      call_detail_preRun_function(DETAIL & detail) { detail.preProcessRun(); }
    };
    // void DETAIL::postProcessRun();
    template <typename DETAIL> no_tag has_detail_postRun_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_postRun_function_helper(detail_void_function<DETAIL, &DETAIL::postProcessRun> *);
    template <typename DETAIL> struct has_detail_postRun_function {
      static bool const value =
        sizeof(has_detail_postRun_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    struct call_detail_postRun_function {
      call_detail_postRun_function(DETAIL & detail) { detail.postProcessRun(); }
    };
    // void DETAIL::endJob();
    template <typename DETAIL> no_tag has_detail_endJob_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_endJob_function_helper(detail_void_function<DETAIL, &DETAIL::endJob> *);
    template <typename DETAIL> struct has_detail_endJob_function {
      static bool const value =
        sizeof(has_detail_endJob_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    struct call_detail_endJob_function {
      call_detail_endJob_function(DETAIL & detail) { detail.endJob(); }
    };
    ////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////
    // Metaprogramming to deal with optional per-provenance functions.
    template <typename DETAIL, void (DETAIL:: *)(Provenance const &)> struct detail_provenance_function;

    template <typename DETAIL>
    struct do_not_call_detail_provenance_function {
      do_not_call_detail_provenance_function(PrincipalProcessor<DETAIL> const &) { }
      template <typename PRINCIPAL>
      void
      operator()(PRINCIPAL const &) { }
    };

    // void DETAIL::processEventProvenance(art:Provenance const &);
    template <typename DETAIL> no_tag has_detail_event_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_event_function_helper(detail_provenance_function<DETAIL, &DETAIL::processEventProvenance> *);
    template <typename DETAIL> struct has_detail_event_function {
      static bool const value =
        sizeof(has_detail_event_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    class call_detail_event_function {
    public:
      call_detail_event_function(PrincipalProcessor<DETAIL> const & pp)
        :
        pp_(pp)
      { }
      template <typename PRINCIPAL>
      void
      operator()(PRINCIPAL const & p) {
        pp_(p, &DETAIL::processEventProvenance);
      }
    private:
      PrincipalProcessor<DETAIL> const & pp_;
    };
    // void DETAIL::processSubRunProvenance(art:Provenance const &);
    template <typename DETAIL> no_tag has_detail_subRun_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_subRun_function_helper(detail_provenance_function<DETAIL, &DETAIL::processSubRunProvenance> *);
    template <typename DETAIL> struct has_detail_subRun_function {
      static bool const value =
        sizeof(has_detail_subRun_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    class call_detail_subRun_function {
    public:
      call_detail_subRun_function(PrincipalProcessor<DETAIL> const & pp)
        :
        pp_(pp)
      { }
      template <typename PRINCIPAL>
      void
      operator()(PRINCIPAL const & p) {
        pp_(p, &DETAIL::processSubRunProvenance);
      }
    private:
      PrincipalProcessor<DETAIL> const & pp_;

    };
    // void DETAIL::processRunProvenance(art:Provenance const &);
    template <typename DETAIL> no_tag has_detail_run_function_helper(...);
    template <typename DETAIL> yes_tag
    has_detail_run_function_helper(detail_provenance_function<DETAIL, &DETAIL::processRunProvenance> *);
    template <typename DETAIL> struct has_detail_run_function {
      static bool const value =
        sizeof(has_detail_run_function_helper<DETAIL>(0)) == sizeof(yes_tag);
    };
    template <typename DETAIL>
    class call_detail_run_function {
    public:
      call_detail_run_function(PrincipalProcessor<DETAIL> const & pp)
        :
        pp_(pp)
      { }
      template <typename PRINCIPAL>
      void
      operator()(PRINCIPAL const & p) {
        pp_(p, &DETAIL::processRunProvenance);
      }
    private:
      PrincipalProcessor<DETAIL> const & pp_;
    };
    ////////////////////////////////////////////////////////////////////////
  }
}

template <typename DETAIL>
void
art::ProvenanceDumper<DETAIL>::
beginJob()
{
  typename std::conditional < detail::has_detail_beginJob_function<DETAIL>::value,
           detail::call_detail_beginJob_function<DETAIL>,
           detail::do_not_call_detail_void_function<DETAIL> >::type maybe_callBeginJob(detail_);
}

template <typename DETAIL>
void
art::ProvenanceDumper<DETAIL>::
write(EventPrincipal const & e)
{
  typename std::conditional < detail::has_detail_preEvent_function<DETAIL>::value,
           detail::call_detail_preEvent_function<DETAIL>,
           detail::do_not_call_detail_void_function<DETAIL> >::type maybe_callPre(detail_);
  typename std::conditional < detail::has_detail_event_function<DETAIL>::value,
           detail::call_detail_event_function<DETAIL>,
           detail::do_not_call_detail_provenance_function<DETAIL> >::type maybe_processPrincipal(pp_);
  maybe_processPrincipal(e);
  typename std::conditional < detail::has_detail_postEvent_function<DETAIL>::value,
           detail::call_detail_postEvent_function<DETAIL>,
           detail::do_not_call_detail_void_function<DETAIL> >::type maybe_callPost(detail_);
}

template <typename DETAIL>
void
art::ProvenanceDumper<DETAIL>::
writeSubRun(SubRunPrincipal const & sr)
{
  typename std::conditional < detail::has_detail_preSubRun_function<DETAIL>::value,
           detail::call_detail_preSubRun_function<DETAIL>,
           detail::do_not_call_detail_void_function<DETAIL> >::type maybe_callPre(detail_);
  typename std::conditional < detail::has_detail_subRun_function<DETAIL>::value,
           detail::call_detail_subRun_function<DETAIL>,
           detail::do_not_call_detail_provenance_function<DETAIL> >::type maybe_processPrincipal(pp_);
  maybe_processPrincipal(sr);
  typename std::conditional < detail::has_detail_postSubRun_function<DETAIL>::value,
           detail::call_detail_postSubRun_function<DETAIL>,
           detail::do_not_call_detail_void_function<DETAIL> >::type maybe_callPost(detail_);
}

template <typename DETAIL>
void
art::ProvenanceDumper<DETAIL>::
writeRun(RunPrincipal const & r)
{
  typename std::conditional < detail::has_detail_preRun_function<DETAIL>::value,
           detail::call_detail_preRun_function<DETAIL>,
           detail::do_not_call_detail_void_function<DETAIL> >::type maybe_callPre(detail_);
  typename std::conditional < detail::has_detail_run_function<DETAIL>::value,
           detail::call_detail_run_function<DETAIL>,
           detail::do_not_call_detail_provenance_function<DETAIL> >::type maybe_processPrincipal(pp_);
  maybe_processPrincipal(r);
  typename std::conditional < detail::has_detail_postRun_function<DETAIL>::value,
           detail::call_detail_postRun_function<DETAIL>,
           detail::do_not_call_detail_void_function<DETAIL> >::type maybe_callPost(detail_);
}

template <typename DETAIL>
void
art::ProvenanceDumper<DETAIL>::
endJob()
{
  typename std::conditional < detail::has_detail_endJob_function<DETAIL>::value,
           detail::call_detail_endJob_function<DETAIL>,
           detail::do_not_call_detail_void_function<DETAIL> >::type maybe_callEndJob(detail_);
}

#endif /* art_Framework_Modules_ProvenanceDumper_h */

// Local Variables:
// mode: c++
// End:
