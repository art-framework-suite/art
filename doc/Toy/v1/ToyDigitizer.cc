#include "ToyDigitizer.hh"

#include "Event.hh"
#include "ToyConfig.hh"
#include "ToyMCHit.hh"
#include "ToyDigi.hh"

#include <vector>
#include <string>
#include <math.h>

using std::vector;
using std::string;

ToyDigitizer::ToyDigitizer( ToyConfig& c ){
  threshold = c.getDouble("threshold");
}

ToyDigitizer::~ToyDigitizer(){
}

void ToyDigitizer::ProcessEvent( Event& evt ){

  // Get the input collection.
  string HitCollectionName = "ToyHits1";
  const vector<ToyMCHit>* hits =
    (const vector<ToyMCHit>*) evt.Get(HitCollectionName);

  // Initialize the output collection.
  vector<ToyDigi>* digis = new vector<ToyDigi>();

  // Loop over inputs
  vector<ToyMCHit>::const_iterator b = hits->begin();
  vector<ToyMCHit>::const_iterator e = hits->end();
  int n(0);
  while (b != e){
    const ToyMCHit& hit = *b;
    ++n;

    // Only make digis if the hit is over threshold.
    if ( hit.pulseHeight() > threshold ){
      digis->push_back( ToyDigi( n, hit, HitCollectionName) );
    }

    ++b;
  }

  // Add this collection to the event.
  string CollectionName = "ToyDigis";
  evt.Put( CollectionName, (void *) digis);

  // Assume that the event looks after the delete.

}
