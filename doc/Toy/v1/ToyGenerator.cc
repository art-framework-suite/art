#include "ToyGenerator.hh"

#include "Event.hh"
#include "ToyConfig.hh"
#include "ToyMCTrack.hh"

#include <vector>
#include <string>
#include <math.h>

using std::vector;
using std::string;

ToyGenerator::ToyGenerator( ToyConfig& c ){

  // Number of tracks to put into each event.
  ntracks = c.getInt("ntracks");

}

ToyGenerator::~ToyGenerator(){
}


void ToyGenerator::ProcessEvent( Event& evt ){

  // Something like this to access the geometry:
  // fwk::ServiceLocator::instance()->find<geom::GeometryManager>() ;

  // Initialize the output collection.
  vector<ToyMCTrack>* v = new vector<ToyMCTrack>();

  for ( int i=0; i<ntracks; ++i ){

    //This is bogus;
    int pdgid = 1;

    // Sine and cosine of polar angle.
    // Randomize this.
    double ct = 0.2;
    double st = sqrt( 1.0 -ct*ct);

    // Azimuth;
    // Randomize this.
    double phi = M_PI/2.0;

    // Mommentum in GeV.
    // Randomize this?
    double p = 0.105;

    // Mass in GeV.
    // Get this from particle data table.
    double m = 0.000511;

    double q[4];
    q[0] = p*st*cos(phi);
    q[1] = p*st*sin(phi);
    q[2] = p*ct;
    q[3] = sqrt( m*m + p*p);

    // Should use the geometry manager to find target locations and
    // pick a random point inside one of the targets.
    double pos[3];
    pos[0] = 0.;
    pos[1] = 0.;
    pos[2] = 0.;

    // Add track to the collection.
    v->push_back( ToyMCTrack(pdgid, q, pos) );

  }

  // Add this collection to the event.
  string CollectionName = "ToyTracks1";
  evt.Put( CollectionName, (void *) v);

  // Assume that the Event looks after the delete.

}
