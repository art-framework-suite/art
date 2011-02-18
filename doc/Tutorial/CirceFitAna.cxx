////////////////////////////////////////////////////////////////////////
/// \file    CirceFit.cxx
/// \brief   Fit vertex and nprongs to event
/// \version $Id: CirceFitAna.cxx,v 1.9 2011/02/01 20:26:40 brebel Exp $
/// \author  messier@indiana.edu
////////////////////////////////////////////////////////////////////////
#include <vector>

// ROOT includes
#include "TH1F.h"

// Framework includes
#include "art/Framework/Core/Event.h"
#include "fhiclcpp/ParameterSet.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "art/Framework/Core/TFileDirectory.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// NOvA includes
#include "TrackFit/CirceFitAna.h"
#include "Geometry/geo.h"
#include "RecoBase/recobase.h"
#include "SimulationBase/MCTruth.h"

namespace trk{

  //......................................................................
  CirceFitAna::CirceFitAna(fhicl::ParameterSet const& pset) : 
     fGenModuleLabel  (pset.get< std::string  >("GenModuleLabel")  )
    ,fProngModuleLabel(pset.get< std::string  >("ProngModuleLabel"))
    ,fFinalVtxX(0)
    ,fFinalVtxY(0)
    ,fFinalVtxZ(0)
  {
  }
  
  //......................................................................
  CirceFitAna::~CirceFitAna()
  {
  }

  //......................................................................
  void CirceFitAna::beginJob()
  {
    art::ServiceHandle<art::TFileService> tfs;

    fFinalVtxX = tfs->make<TH1F>("fFinalVtxX",
				 "Circe Vertex X; Xfit-Xmc (cm); Events",
				 200, -50.0,50.0);
    fFinalVtxY = tfs->make<TH1F>("fFinalVtxY",
				 "Circe Vertex Y; Yfit-Ymc (cm); Events",
				 200, -50.0,50.0);
    fFinalVtxZ = tfs->make<TH1F>("fFinalVtxZ",
				 "Circe Vertex Z; Zfit-Zmc (cm); Events",
				 200, -50.0,50.0);

    return;
  }

  //......................................................................
  void CirceFitAna::analyze(art::Event const& evt) 
  {

    // The Monte Carlo true 4-vectors for the event
    art::Handle< std::vector<simb::MCTruth> > mctruths;
    evt.getByLabel(fGenModuleLabel, mctruths);

    // We're only set up to handle one MCTruth object at a time...
    if (mctruths->size()==0) {
      mf::LogWarning("CirceFitAna:TooLittleTruth") << "too few mctruth objects, bail";
      return;
    }
    else if (mctruths->size()>1) {
      mf::LogWarning("CirceFitAna:TooMuchTruth") << __FILE__ << ":" << __LINE__ 
		<< " Not ready for overlap events yet..." << std::endl;
      return;
    }

    art::Ptr<simb::MCTruth> mctruth(mctruths, 0);

    const simb::MCNeutrino nu = mctruth->GetNeutrino();
    if (!mctruth->NeutrinoSet() ) {
      throw art::Exception(art::errors::ProductNotFound) << "CirceFitAna::analyze No neutrino??";
    }
    
    // Plot where the true vertex is compared to where we are seeding
    double x0mc = nu.Lepton().Vx();
    double y0mc = nu.Lepton().Vy();
    double z0mc = nu.Lepton().Vz();
    
    // Pull the prongs we fit out of the event
    art::Handle< std::vector<rb::Prong> > prongs;
    evt.getByLabel(fGenModuleLabel, prongs);

    if (prongs->size()>1 && nu.CCNC()==simb::kCC) {
      
      art::Ptr<rb::Prong> prong(prongs,0);

      fFinalVtxX->Fill(prong->StartPos()[0]-x0mc);
      fFinalVtxY->Fill(prong->StartPos()[1]-y0mc);
      fFinalVtxZ->Fill(prong->StartPos()[2]-z0mc);
    }
    
    return;
  }
}//namespace 

////////////////////////////////////////////////////////////////////////

