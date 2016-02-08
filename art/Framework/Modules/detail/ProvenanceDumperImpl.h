#ifndef art_Framework_Modules_detail_ProvenanceDumperImpl_h
#define art_Framework_Modules_detail_ProvenanceDumperImpl_h
////////////////////////////////////////////////////////////////////////
// ProvenanceDumperImpl
//
// Provides main implementation for ProvenanceDumper
//
// Uses A LOT of metaprogramming
/////////////////////////////////////////
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
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "canvas/Utilities/detail/metaprogramming.h"
#include "fhiclcpp/ParameterSet.h"

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

    // The function that does all the work: everything else is fluff.
    template <typename DETAIL>
    template <typename PRINCIPAL>
    void
    PrincipalProcessor<DETAIL>::
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

    template <typename DETAIL>
    class ProvenanceDumperImpl {

      DETAIL & detail_;
      PrincipalProcessor<DETAIL> & pp_;

    public:

      ProvenanceDumperImpl(DETAIL & detail,
                           PrincipalProcessor<DETAIL> & pp)
        : detail_{detail}
        , pp_{pp}
      {}

      void beginJob()
      {
        std::conditional_t< has_detail_beginJob_function<DETAIL>::value,
          call_detail_beginJob_function<DETAIL>,
          do_not_call_detail_void_function<DETAIL> > maybe_callBeginJob(detail_);
      }

      void write(EventPrincipal & e)
      {
        std::conditional_t< has_detail_preEvent_function<DETAIL>::value,
          call_detail_preEvent_function<DETAIL>,
          do_not_call_detail_void_function<DETAIL> > maybe_callPre(detail_);
        std::conditional_t< has_detail_event_function<DETAIL>::value,
          call_detail_event_function<DETAIL>,
          do_not_call_detail_provenance_function<DETAIL> > maybe_processPrincipal(pp_);
        maybe_processPrincipal(e);
        std::conditional_t< has_detail_postEvent_function<DETAIL>::value,
          call_detail_postEvent_function<DETAIL>,
          do_not_call_detail_void_function<DETAIL> > maybe_callPost(detail_);
      }

      void writeSubRun(SubRunPrincipal & sr)
      {
        std::conditional_t< has_detail_preSubRun_function<DETAIL>::value,
          call_detail_preSubRun_function<DETAIL>,
          do_not_call_detail_void_function<DETAIL> > maybe_callPre(detail_);
        std::conditional_t< has_detail_subRun_function<DETAIL>::value,
          call_detail_subRun_function<DETAIL>,
          do_not_call_detail_provenance_function<DETAIL> > maybe_processPrincipal(pp_);
        maybe_processPrincipal(sr);
        std::conditional_t< has_detail_postSubRun_function<DETAIL>::value,
          call_detail_postSubRun_function<DETAIL>,
          do_not_call_detail_void_function<DETAIL> > maybe_callPost(detail_);
      }

      void writeRun(RunPrincipal & r)
      {
        std::conditional_t< has_detail_preRun_function<DETAIL>::value,
          call_detail_preRun_function<DETAIL>,
          do_not_call_detail_void_function<DETAIL> > maybe_callPre(detail_);
      std::conditional_t< has_detail_run_function<DETAIL>::value,
          call_detail_run_function<DETAIL>,
          do_not_call_detail_provenance_function<DETAIL> > maybe_processPrincipal(pp_);
      maybe_processPrincipal(r);
      std::conditional_t< has_detail_postRun_function<DETAIL>::value,
          call_detail_postRun_function<DETAIL>,
          do_not_call_detail_void_function<DETAIL> > maybe_callPost(detail_);
      }

      void endJob()
      {
        std::conditional_t< has_detail_endJob_function<DETAIL>::value,
          call_detail_endJob_function<DETAIL>,
          do_not_call_detail_void_function<DETAIL> > maybe_callEndJob(detail_);
      }
    };


  } // detail
} // art

#endif /* art_Framework_Modules_ProvenanceDumperImpl_h */

// Local Variables:
// mode: c++
// End:
