///////////////////////////////////////////////////////////////////////
/// \file    CirceFit.h
/// \brief   Track fitter
/// \author  messier@indiana.edu
/// \version $Id: CirceFitAna.h,v 1.3 2011/01/20 16:43:21 p-novaart Exp $
///////////////////////////////////////////////////////////////////////
#ifndef TRACKFIT_CIRCEFIT_H
#define TRACKFIT_CIRCEFIT_H
#include <string>

#include "art/Framework/Core/EDAnalyzer.h"

class TH1F;

namespace trk {
  class CirceFitAna : public art::EDAnalyzer {
  public:
    explicit CirceFitAna(fhicl::ParameterSet const& pset);
    virtual ~CirceFitAna();

    void beginJob();
    void analyze(art::Event const& evt);

  private:
    std::string  fGenModuleLabel;   ///< Where to find simb::MCTruth to reconstruct
    std::string  fProngModuleLabel; ///< Where to find rb::Prongs to reconstruct

    TH1F* fFinalVtxX; ///< X vertex resolution
    TH1F* fFinalVtxY; ///< Y vertex resolution
    TH1F* fFinalVtxZ; ///< Z vertex resolution

  };
}
#endif
