///////////////////////////////////////////////////////////////////////
/// \file    CirceFit.h
/// \brief   Track fitter
/// \author  messier@indiana.edu
/// \version $Id: CirceFit.h,v 1.3 2011/01/20 16:43:21 p-novaart Exp $
///////////////////////////////////////////////////////////////////////
#ifndef TRACKFIT_CIRCEFIT_H
#define TRACKFIT_CIRCEFIT_H
#include <string>

#include "art/Framework/Core/EDProducer.h"

class TH1F;

namespace trk {
  class CirceFit : public art::EDProducer {
  public:
    explicit CirceFit(fhicl::ParameterSet const& pset);
    virtual ~CirceFit();

    void produce(art::Event& evt);

  private:
    int          fDebug;        ///< Controls debug output 0=none, 99=scream
    std::string  fInputCalHits; ///< Where to find CalHits to reconstruct
    unsigned int fMinXhits;     ///< Required # of hits in x view
    unsigned int fMinYhits;     ///< Required # of hits in x view
    int          fNmax;         ///< Hard stop after this many prongs
    double       fStop;         ///< Stopping criteria
    int          fWeightMethod; ///< What method to use to weight hits
    double       fOtherViewW0;  ///< Weight for NN hits in other view
    double       fNNZrange;     ///< Range in z for NN weight
    double       fNNZedge;      ///< Width of edge for NN weight
    double       fNNTrange;     ///< Rnage in x or y for NN weight
    double       fNNTedge;      ///< Width of edge for NN weight

  private:

    TH1F* fFinalVtxX; ///< X vertex resolution
    TH1F* fFinalVtxY; ///< Y vertex resolution
    TH1F* fFinalVtxZ; ///< Z vertex resolution

  };
}
#endif
