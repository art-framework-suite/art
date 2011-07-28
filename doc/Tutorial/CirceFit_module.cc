// declaring and register a module
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Core/EDFilter.h"
#include "fhiclcpp/ParameterSet.h"
// event interactions
#include "art/Framework/Principal/Event.h"
#include "art/Persistency/Common/Handle.h"
#include "art/Persistency/Common/PtrVector.h"
#include "art/Persistency/Common/Ptr.h"
// service interactions
#include "art/Framework/Services/Registry/ServiceHandle.h"
#include "art/Framework/Services/Optional/TFileService.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

// experiment specific headers go here
#include ...

namespace trk {
  class CirceFit : public art::EDFilter
  {
  public:
    explicit CirceFit(fhicl::ParameterSet const& pset);
    virtual ~CirceFit();

    bool filter(art::Event& evt);

  private:
    typedef std::vector<rb::Prong> ProngList;

    TH1F* z; ///< Z vertex resolution
  };
}

namespace trk{

  CirceFit::CirceFit(fhicl::ParameterSet const& pset) :
    z()
  {
    produces<ProngList>();
  }

  // fInputCalHits(pset.get< std::string  >("InputCalHits"))

  CirceFit::~CirceFit()
  {
    // in a well-designed module, this should be empty!
    // there is a document on the suggested use of pointers at
    //   https://cdcvs.fnal.gov/redmine/documents/184
  }

  bool CirceFit::filter(art::Event& evt)
  {
    // Pull the calibrated hits out of the event
    typedef std::vector<rb::CellHit> input_t;
    art::Handle<input_t> hitcol;
    evt.getByLabel(fInputCalHits, hitcol);

    // make art::PtrVector of the hits
    art::PtrVector<rb::CellHit> hits;
    for(unsigned int i = 0; i < hitcol->size(); ++i)
      {
        art::Ptr<rb::CellHit> hit(hitcol,i);
        hits.push_back(hit);
      }

    art::ServiceHandle<geo::Geometry> geom;
    trk::Measurement m;

    // Select hits to try to reconstruct and assign them weights
    unsigned int nxview = 0;
    unsigned int nyview = 0;
    double xyz[3];

    for(input_t::const_iterator hit=hitcol.begin(),end=hitcol.end();
        hit!=end;++it)
      {
        // Assign weight to hit.
        double w = fWeightMethod==1 ? hit->PE() : 1.0;

        const geo::CellGeo* cell = geom->Plane(hit->Plane())->Cell(hit->Cell());
        // Get the location of the center of the cell
        cell->GetCenter(xyz, 0.0);
        // measurement needs to grow a member function to simplify these actions...
        m.addHitInformation(*hit, xyz,w);

        // view is an enum
        if (hit->View()==geo::kX) ++nxview;
        if (hit->View()==geo::kY) ++nyview;
      }

    //m.fHits.push_back(hits[i]);
    //m.fX.push_back(xyz[0]);
    //m.fY.push_back(xyz[1]);
    //m.fZ.push_back(xyz[2]);
    //m.fW.push_back(w);

    // Only bother if we have some minimum number of hits in each view
    // JBK - is this an error, or just stopping further processing?
    // this looks like a filter condition and looks like an example of a producing filter.
    // in this case, a LogInfo message would help inform the person running the program
    // of the particilar condition.

    if (nxview<fMinXhits || nyview<fMinYhits)
      {
          LogInfo("NotEnoughHits") << "too few hits, x: " << nxview << " y: " << nyview;
          return false;
      }

    // Build the fitter and pass it the data to fit
    Circe fitter;

    for (double stdev=fStop, int i=1; i<=fNmax && stdev>=fStop; ++i)
      {
        stdev = fitter.Fit(i,&m);
        LOG_DEBUG("CirceFit:stdev") << i << " stdev=" << stdev << std::endl;
      }

    std::auto_ptr<ProngList> prongcol( new ProngList );
    fitter.MakeProngs(*prongcol);
    evt.put(prongcol);
    return true;
  }

}

namespace trk{
  DEFINE_ART_MODULE(CirceFit);
}
