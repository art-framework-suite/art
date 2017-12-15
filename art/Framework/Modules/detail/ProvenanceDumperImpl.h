#ifndef art_Framework_Modules_detail_ProvenanceDumperImpl_h
#define art_Framework_Modules_detail_ProvenanceDumperImpl_h
////////////////////////////////////////////////////////////////////////
// ProvenanceDumperImpl
//
// Provides main implementation for ProvenanceDumper
//
// Uses A LOT of metaprogramming.
//
// The process_() function will loop over the Groups in a particular
// Principal, attempt to resolve the product if possible and then
// construct a Provenance to pass to the correct callback function func.
//
////////////////////////////////////////////////////////////////////////
#include "art/Framework/Principal/EventPrincipal.h"
#include "art/Framework/Principal/Provenance.h"
#include "art/Framework/Principal/RunPrincipal.h"
#include "art/Framework/Principal/SubRunPrincipal.h"
#include "cetlib/metaprogramming.h"
#include "fhiclcpp/ParameterSet.h"

namespace art {
  namespace detail {

    template <typename DETAIL>
    class PrincipalProcessor {
    public:
      PrincipalProcessor(DETAIL& detail,
                         bool const wantPresentOnly,
                         bool const resolveProducts,
                         bool const wantResolvedOnly)
        : detail_(detail)
        , wantPresentOnly_(wantPresentOnly)
        , resolveProducts_(resolveProducts)
        , wantResolvedOnly_(wantResolvedOnly)
      {}

      void operator()(art::Principal const& p,
                      void (DETAIL::*func)(art::Provenance const&)) const;

    private:
      DETAIL& detail_;
      bool const wantPresentOnly_;
      bool const resolveProducts_;
      bool const wantResolvedOnly_;
    };

    template <typename DETAIL>
    void
    PrincipalProcessor<DETAIL>::operator()(
      art::Principal const& p,
      void (DETAIL::*func)(art::Provenance const&)) const
    {
      for (auto const& pr : p) {
        Group const& g = *pr.second;
        if (resolveProducts_) {
          try {
            if (!g.resolveProduct(g.producedWrapperType())) {
              throw Exception(errors::DataCorruption, "data corruption");
            }
          }
          catch (art::Exception const& e) {
            if (e.category() != "ProductNotFound") {
              throw;
            }
            if (g.anyProduct())
              throw art::Exception(errors::LogicError, "ProvenanceDumper", e)
                << "Product reported as not present, but is pointed to "
                   "nonetheless!";
          }
        }
        bool wantCallFunc = true;
        Provenance const prov{cet::make_exempt_ptr(&g)};
        if (wantResolvedOnly_) {
          wantCallFunc = (g.anyProduct() != nullptr);
        } else if (wantPresentOnly_) {
          // Unfortunately, there are files in which the product
          // provenance has not been appropriately stored for dropped
          // products.  The first check below on the product
          // provenance pointer is a precondition to calling
          // prov.isPresent(), getting around this incorrect
          // persistency behavior.
          wantCallFunc =
            (g.productProvenancePtr() != nullptr) && prov.isPresent();
        }

        if (wantCallFunc) {
          (detail_.*func)(prov);
        }
      }
    }

    ////////////////////////////////////////////////////////////////////////
    // Metaprogramming to provide default function if optional
    // functions do not exist.

    template <typename T>
    struct default_invocation;

    template <typename R, typename... ARGS>
    struct default_invocation<R(ARGS...)> {
      static R
      invoke(ARGS...)
      {}
    };

    ////////////////////////////////////////////////////////////////////////
    // Metaprogramming to deal with optional pre/post and begin/end
    // job functions.
    using cet::enable_if_function_exists_t;

    // void DETAIL::beginJob();
    template <typename DETAIL, typename Enable = void>
    struct maybe_beginJob : default_invocation<void(DETAIL&)> {
    };

    template <typename DETAIL>
    struct maybe_beginJob<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(), &DETAIL::beginJob>> {
      static void
      invoke(DETAIL& detail)
      {
        detail.beginJob();
      }
    };

    // void DETAIL::preProcessEvent();
    template <typename DETAIL, typename Enable = void>
    struct maybe_preProcessEvent : default_invocation<void(DETAIL&)> {
    };

    template <typename DETAIL>
    struct maybe_preProcessEvent<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(),
                                  &DETAIL::preProcessEvent>> {
      static void
      invoke(DETAIL& detail)
      {
        detail.preProcessEvent();
      }
    };

    // void DETAIL::postProcessEvent();
    template <typename DETAIL, typename Enable = void>
    struct maybe_postProcessEvent : default_invocation<void(DETAIL&)> {
    };

    template <typename DETAIL>
    struct maybe_postProcessEvent<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(),
                                  &DETAIL::postProcessEvent>> {
      static void
      invoke(DETAIL& detail)
      {
        detail.postProcessEvent();
      }
    };

    // void DETAIL::preProcessSubRun();
    template <typename DETAIL, typename Enable = void>
    struct maybe_preProcessSubRun : default_invocation<void(DETAIL&)> {
    };

    template <typename DETAIL>
    struct maybe_preProcessSubRun<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(),
                                  &DETAIL::preProcessSubRun>> {
      static void
      invoke(DETAIL& detail)
      {
        detail.preProcessSubRun();
      }
    };

    // void DETAIL::postProcessSubRun();
    template <typename DETAIL, typename Enable = void>
    struct maybe_postProcessSubRun : default_invocation<void(DETAIL&)> {
    };

    template <typename DETAIL>
    struct maybe_postProcessSubRun<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(),
                                  &DETAIL::postProcessSubRun>> {
      static void
      invoke(DETAIL& detail)
      {
        detail.postProcessSubRun();
      }
    };

    // void DETAIL::preProcessRun();
    template <typename DETAIL, typename Enable = void>
    struct maybe_preProcessRun : default_invocation<void(DETAIL&)> {
    };

    template <typename DETAIL>
    struct maybe_preProcessRun<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(), &DETAIL::preProcessRun>> {
      static void
      invoke(DETAIL& detail)
      {
        detail.preProcessRun();
      }
    };

    // void DETAIL::postProcessRun();
    template <typename DETAIL, typename Enable = void>
    struct maybe_postProcessRun : default_invocation<void(DETAIL&)> {
    };

    template <typename DETAIL>
    struct maybe_postProcessRun<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(),
                                  &DETAIL::postProcessRun>> {
      static void
      invoke(DETAIL& detail)
      {
        detail.postProcessRun();
      }
    };

    // void DETAIL::endJob();
    template <typename DETAIL, typename Enable = void>
    struct maybe_endJob : default_invocation<void(DETAIL&)> {
    };

    template <typename DETAIL>
    struct maybe_endJob<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(), &DETAIL::endJob>> {
      static void
      invoke(DETAIL& detail)
      {
        detail.endJob();
      }
    };
    ////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////
    // Metaprogramming to deal with optional per-provenance functions.

    // void DETAIL::processSubRunProvenance(art:Provenance const &);
    template <typename DETAIL, typename Enable = void>
    struct maybe_processEventPrincipal
      : default_invocation<void(PrincipalProcessor<DETAIL> const&,
                                EventPrincipal const&)> {
    };

    template <typename DETAIL>
    struct maybe_processEventPrincipal<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(Provenance const&),
                                  &DETAIL::processEventProvenance>> {
      static void
      invoke(PrincipalProcessor<DETAIL> const& pp, EventPrincipal const& p)
      {
        pp(p, &DETAIL::processEventProvenance);
      }
    };

    // void DETAIL::processSubRunProvenance(art:Provenance const &);
    template <typename DETAIL, typename Enable = void>
    struct maybe_processSubRunPrincipal
      : default_invocation<void(PrincipalProcessor<DETAIL> const&,
                                SubRunPrincipal const&)> {
    };

    template <typename DETAIL>
    struct maybe_processSubRunPrincipal<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(Provenance const&),
                                  &DETAIL::processSubRunProvenance>> {
      static void
      invoke(PrincipalProcessor<DETAIL> const& pp, SubRunPrincipal const& p)
      {
        pp(p, &DETAIL::processSubRunProvenance);
      }
    };

    // void DETAIL::processRunProvenance(art:Provenance const &);
    template <typename DETAIL, typename Enable = void>
    struct maybe_processRunPrincipal
      : default_invocation<void(PrincipalProcessor<DETAIL> const&,
                                RunPrincipal const&)> {
    };

    template <typename DETAIL>
    struct maybe_processRunPrincipal<
      DETAIL,
      enable_if_function_exists_t<void (DETAIL::*)(Provenance const&),
                                  &DETAIL::processRunProvenance>> {
      static void
      invoke(PrincipalProcessor<DETAIL> const& pp, RunPrincipal const& p)
      {
        pp(p, &DETAIL::processRunProvenance);
      }
    };

    ////////////////////////////////////////////////////////////////////////

    template <typename DETAIL>
    class ProvenanceDumperImpl {

      DETAIL& detail_;
      PrincipalProcessor<DETAIL>& pp_;

    public:
      ProvenanceDumperImpl(DETAIL& detail, PrincipalProcessor<DETAIL>& pp)
        : detail_{detail}, pp_{pp}
      {}

      void
      beginJob()
      {
        maybe_beginJob<DETAIL>::invoke(detail_);
      }

      void
      write(EventPrincipal& e)
      {
        maybe_preProcessEvent<DETAIL>::invoke(detail_);
        maybe_processEventPrincipal<DETAIL>::invoke(pp_, e);
        maybe_postProcessEvent<DETAIL>::invoke(detail_);
      }

      void
      writeSubRun(SubRunPrincipal& sr)
      {
        maybe_preProcessSubRun<DETAIL>::invoke(detail_);
        maybe_processSubRunPrincipal<DETAIL>::invoke(pp_, sr);
        maybe_postProcessSubRun<DETAIL>::invoke(detail_);
      }

      void
      writeRun(RunPrincipal& r)
      {
        maybe_preProcessRun<DETAIL>::invoke(detail_);
        maybe_processRunPrincipal<DETAIL>::invoke(pp_, r);
        maybe_postProcessRun<DETAIL>::invoke(detail_);
      }

      void
      endJob()
      {
        maybe_endJob<DETAIL>::invoke(detail_);
      }
    };

  } // detail
} // art

#endif /* art_Framework_Modules_detail_ProvenanceDumperImpl_h */

// Local Variables:
// mode: c++
// End:
