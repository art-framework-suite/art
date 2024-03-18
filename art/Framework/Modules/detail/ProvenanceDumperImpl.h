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
#include "canvas/Utilities/Exception.h"
#include "fhiclcpp/ParameterSet.h"

namespace art::detail {

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
        bool const resolved_product = g.resolveProductIfAvailable();
        if (!resolved_product) {
          continue;
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
        wantCallFunc = (g.productProvenance() != nullptr) && prov.isPresent();
      }

      if (wantCallFunc) {
        (detail_.*func)(prov);
      }
    }
  }

  ////////////////////////////////////////////////////////////////////////
  // Metaprogramming to deal with optional pre/post and begin/end
  // job functions.

  // void DETAIL::beginJob();
  template <typename T>
  concept maybe_beginJob = requires(T& t) {
                             {
                               t.beginJob()
                             };
                           };

  // void DETAIL::preProcessEvent();
  template <typename T>
  concept maybe_preProcessEvent = requires(T& t) {
                                    {
                                      t.preProcessEvent()
                                    };
                                  };

  // void DETAIL::postProcessEvent();
  template <typename T>
  concept maybe_postProcessEvent = requires(T& t) {
                                     {
                                       t.postProcessEvent()
                                     };
                                   };

  // void DETAIL::preProcessSubRun();
  template <typename T>
  concept maybe_preProcessSubRun = requires(T& t) {
                                     {
                                       t.preProcessSubRun()
                                     };
                                   };

  // void DETAIL::postProcessSubRun();
  template <typename T>
  concept maybe_postProcessSubRun = requires(T& t) {
                                      {
                                        t.postProcessSubRun()
                                      };
                                    };

  // void DETAIL::preProcessRun();
  template <typename T>
  concept maybe_preProcessRun = requires(T& t) {
                                  {
                                    t.preProcessRun()
                                  };
                                };

  // void DETAIL::postProcessRun();
  template <typename T>
  concept maybe_postProcessRun = requires(T& t) {
                                   {
                                     t.postProcessRun()
                                   };
                                 };

  // void DETAIL::endJob();
  template <typename T>
  concept maybe_endJob = requires(T& t) {
                           {
                             t.endJob()
                           };
                         };
  ////////////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////////////
  // Metaprogramming to deal with optional per-provenance functions.

  // void DETAIL::processEventProvenance(art:Provenance const &);
  template <typename T>
  concept maybe_processEventPrincipal = requires(T& t, art::Provenance& p) {
                                          {
                                            t.processEventProvenance(p)
                                          };
                                        };

  // void DETAIL::processSubRunProvenance(art:Provenance const &);
  template <typename T>
  concept maybe_processSubRunPrincipal = requires(T& t, art::Provenance& p) {
                                           {
                                             t.processSubRunProvenance(p)
                                           };
                                         };

  // void DETAIL::processRunProvenance(art:Provenance const &);
  template <typename T>
  concept maybe_processRunPrincipal = requires(T& t, art::Provenance& p) {
                                        {
                                          t.processRunProvenance(p)
                                        };
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
      if constexpr (maybe_beginJob<DETAIL>) {
        detail_.beginJob();
      }
    }

    void
    write(EventPrincipal& e)
    {
      if constexpr (maybe_preProcessEvent<DETAIL>) {
        detail_.preProcessEvent();
      }
      if constexpr (maybe_processEventPrincipal<DETAIL>) {
        pp_(e, &DETAIL::processEventProvenance);
      }
      if constexpr (maybe_postProcessEvent<DETAIL>) {
        detail_.postProcessEvent();
      }
    }

    void
    writeSubRun(SubRunPrincipal& sr)
    {
      if constexpr (maybe_preProcessSubRun<DETAIL>) {
        detail_.preProcessSubRun();
      }
      if constexpr (maybe_processSubRunPrincipal<DETAIL>) {
        pp_(sr, &DETAIL::processSubRunProvenance);
      }
      if constexpr (maybe_postProcessSubRun<DETAIL>) {
        detail_.postProcessSubRun();
      }
    }

    void
    writeRun(RunPrincipal& r)
    {
      if constexpr (maybe_preProcessRun<DETAIL>) {
        detail_.preProcessRun();
      }
      if constexpr (maybe_processRunPrincipal<DETAIL>) {
        pp_(r, &DETAIL::processRunProvenance);
      }
      if constexpr (maybe_postProcessRun<DETAIL>) {
        detail_.postProcessRun();
      }
    }

    void
    endJob()
    {
      if constexpr (maybe_endJob<DETAIL>) {
        detail_.endJob();
      }
    }
  };

} // namespace art::detail

#endif /* art_Framework_Modules_detail_ProvenanceDumperImpl_h */

// Local Variables:
// mode: c++
// End:
