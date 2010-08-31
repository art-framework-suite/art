#include "ToyG4.hh"
#include "Event.hh"
#include "ToyConfig.hh"

#include "ToyMCTrack.hh"
#include "ToyMCHit.hh"

#include <vector>
#include <string>
#include <math.h>

using std::vector;
using std::string;

ToyG4::ToyG4( ToyConfig& c ){

  nhits = c.getInt("nhits");

  // Instantiate the G4 geometry here or in the one of the
  // Begin* methods of this class.
  //
  // Something like the following to access the geometry manager.
  // fwk::ServiceLocator::instance()->find<geom::GeometryManager>() ;
  //

}

ToyG4::~ToyG4(){
}



void ToyG4::ProcessEvent( Event& evt ){

  // Get the required inputs.
  string TrackCollectionName = "ToyTracks1";
  const vector<ToyMCTrack>* tracks =
    (const vector<ToyMCTrack>*) evt.Get(TrackCollectionName);

  // Intialize the output collections;
  vector<ToyMCHit>* hits = new vector<ToyMCHit>();
  vector<ToyMCHit>* hits2 = new vector<ToyMCHit>();

  // This is where we would really run G4 to make hits.
  // For this toy model lets just put some silly hits on the tracks.

  // Loop over the inputs.
  int n(0);
  vector<ToyMCTrack>::const_iterator b = tracks->begin();
  vector<ToyMCTrack>::const_iterator e = tracks->end();
  while (b != e){
    const ToyMCTrack& track = *b;
    ++n;

    // Add some number of hits for each track.
    int channel(0);
    for ( int i=0; i<nhits; ++i){
      ++channel;
      hits->push_back( ToyMCHit( n, channel, 1., 2.) );
    }

    ++b;
  }

  // Add this collection to the event.
  string CollectionName = "ToyHits1";
  evt.Put( CollectionName, (void *) hits);

  // The second collection just happens to be empty.
  string CollectionName2 = "ToyHits2";
  evt.Put( CollectionName2, (void *) hits2);

  // Assume that the event looks after the delete.

}
