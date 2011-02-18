////////////////////////////////////////////////////////////////////////
/// \file    CirceFit.cxx
/// \brief   Fit vertex and nprongs to event
/// \version $Id: CirceFit.cxx,v 1.9 2011/02/01 20:27:52 brebel Exp $
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
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/Ptr.h"

// NOvA includes
#include "TrackFit/CirceFit.h"
#include "TrackFit/Circe.h"
#include "Geometry/geo.h"
#include "RecoBase/recobase.h"
#include "SimulationBase/MCTruth.h"

namespace trk{

  //......................................................................
  CirceFit::CirceFit(fhicl::ParameterSet const& pset) : 
    fDebug        (pset.get< int          >("Debug")       )
    ,fInputCalHits(pset.get< std::string  >("InputCalHits"))
    ,fMinXhits    (pset.get< unsigned int >("MinXhits")    )
    ,fMinYhits    (pset.get< unsigned int >("MinYhits")    )
    ,fNmax        (pset.get< int          >("Nmax")        )
    ,fStop        (pset.get< double       >("Stop")        )
    ,fWeightMethod(pset.get< int          >("WeightMethod"))
    ,fOtherViewW0 (pset.get< double       >("OtherViewW0") )
    ,fNNZrange    (pset.get< double       >("NNZrange")    )
    ,fNNZedge     (pset.get< double       >("NNZedge")     )
    ,fNNTrange    (pset.get< double       >("NNTrange")    )
    ,fNNTedge     (pset.get< double       >("NNTedge")     )
  {
    produces< std::vector<rb::Prong> >();
  }

  //......................................................................
  CirceFit::~CirceFit()
  {
  }

  //......................................................................
  void CirceFit::produce(art::Event& evt) 
  {
    // Pull the calibrated hits out of the event
    art::Handle< std::vector<rb::CellHit> > hitcol;
    evt.getByLabel(fInputCalHits, hitcol);

    //make art::PtrVector of the hits
    art::PtrVector<rb::CellHit> hits;
    for(unsigned int i = 0; i < hitcol->size(); ++i){
      art::Ptr<rb::CellHit> hit(hitcol,i);
      hits.push_back(hit);
    }
    
    if (fDebug>0) {
      mf::LogWarning("CirceFit:Size") << " Found " << hits.size() << " hits in event.";
    }

    art::ServiceHandle<geo::Geometry> geom;

    // Select hits to try to reconstruct and assign them weights
    unsigned int nxview = 0;
    unsigned int nyview = 0;
    trk::Measurement m;
    for (unsigned int i=0; i<hits.size(); ++i) {
      // Assign weight to hit.
      double w = 0.0;
      switch (fWeightMethod) {
      case 1:
	w = hits[i]->PE();
	break;
      default:
	w = 1.0;
	break;
      }
      if (fDebug>0) mf::LogWarning("CirceFit:Weight") << "Weight=" << w << std::endl;
    
      // Get the location of the center of the cell
      double xyz[3];
      const geo::CellGeo* cell = geom->Plane(hits[i]->Plane())->Cell(hits[i]->Cell());
      cell->GetCenter(xyz, 0.0);

      m.fHits.push_back(hits[i]);
      m.fX.push_back(xyz[0]);
      m.fY.push_back(xyz[1]);
      m.fZ.push_back(xyz[2]);
      m.fW.push_back(w);
    
      if (hits[i]->View()==geo::kX) ++nxview;
      if (hits[i]->View()==geo::kY) ++nyview;
    }

    // Only bother if we have some minimum number of hits in each view
    if (nxview<fMinXhits || nyview<fMinYhits){
      throw art::Exception(art::errors::StdException) << "too few hits, x: " << nxview << " y: " << nyview;

      return;
    }

    // Build the fitter and pass it the data to fit
    double stdev;
    Circe fitter;
    for (int i=1; i<=fNmax; ++i) {
      stdev = fitter.Fit(i,&m);
      if (fDebug>0) {
	mf::LogWarning("CirceFit:stdev") << i << " stdev=" << stdev << std::endl;
      }
      if (stdev<fStop) break;
    }

    std::vector<rb::Prong> prong;
    fitter.MakeProngs(prong);
  
    std::auto_ptr< std::vector<rb::Prong> > prongcol( new std::vector<rb::Prong>(prong) );

    evt.put(prongcol);

    return;
  }

}//namespace
////////////////////////////////////////////////////////////////////////

